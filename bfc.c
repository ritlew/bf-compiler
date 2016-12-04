#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/wait.h>

int only_asm = 0;
int only_object = 0;

void create_exe(int buf_n, char ** buffer, char * filename){
    int pid;

    char temp[1000];
    int i;
    FILE* f;
	f = fopen("prog.asm", "w");

		char * beg = "section	.text\n\
\tglobal _start\n\
\n\
section    .data\n\
\n\
\tbuf times 30000 dd 0\n\
\tbuf_p dd 0\n";

	fwrite(beg, strlen(beg), 1, f);
	fwrite("_start:\n", strlen("_start:\n"), 1, f);
	for (i = 0; i < buf_n; i++){
		sprintf(temp, "\t%s", buffer[i]);
		fwrite(temp, strlen(temp), 1, f);
	}
	char * end = "\tmov eax, 1\n\tmov ebx, 0\n\tint 0x80";
	fwrite(end, strlen(end), 1, f);

	fclose(f);
	if (!only_asm){

	    pid = fork();

		if (pid){
			wait(NULL);
			unlink("prog.asm");
			if (!only_object){
				unlink("prog.o");
			}

		} else {
			pid = fork();

		    if (pid){
		    	wait(NULL);
		    	if (!only_object){
			    	char * commands[7] = {"ld", "-m", "elf_i386", "-o", "prog", "prog.o", NULL};
					execvp(commands[0], commands);
				} else {
					exit(0);
				}
		    } else {
		    	char * commands[5] = {"nasm", "-f", "elf", "prog.asm", NULL};
				execvp(commands[0], commands);
		    }
			
		}
	}

}

void buf_write(int* buf_n, char * str, char ** buffer){
	buffer[*buf_n] = (char *)malloc(strlen(str));
	strcpy(buffer[(*buf_n)++], str);
}

int main(int argc, char* argv[]){
	if (argc < 2){
		printf("No input specified\n");
		return -1;
	}

	int opt;

	while ((opt = getopt(argc, argv, "SO")) != -1){
		switch (opt){
			case 'S': only_asm = 1; break;
			case 'O': only_object = 1; break;
			default: ;
		}
	}

	int i;
	char * file[10000];



	char * buffer[30000];
	int buf_n = 0;

	int buf_p = 0;
	int labels[500] = {0};
	int labels_d[500] = {0};
	int label_l = 0;
	char read_buffer[30001];

	FILE* read_f;
	read_f = fopen(argv[argc-1], "r");

	int t = fread(read_buffer, 1, 30000, read_f);
	int r_ptr = 0;
	for (i = 0; i < t; i++){
		if (strchr(".+-<>[]", read_buffer[i])){
			read_buffer[r_ptr++] = read_buffer[i];
		}
	}

	fclose(read_f);

	read_buffer[r_ptr] = '\0';

	char temp[80];
	int a_count;
	buf_write(&buf_n, "mov ebx, 1\n", buffer);
	buf_write(&buf_n, "mov eax, buf_p\n", buffer);
	buf_write(&buf_n, "mov ecx, buf\n", buffer);
	buf_write(&buf_n, "add ecx, [buf_p]\n\n", buffer);
	for (i = 0; i < strlen(read_buffer); i++){
		if (read_buffer[i] == '+'){
			a_count = 1;
			while (read_buffer[++i] == '+'){
				a_count++;
			} 
			i--;

			//buf_write(&buf_n, "mov eax, buf_p\n", buffer);
			///buf_write(&buf_n, "mov ecx, buf\n", buffer);
			//buf_write(&buf_n, "add ecx, [buf_p]\n", buffer);
			//buf_write(&buf_n, "mov ebx, 1\n", buffer);
			if (a_count > 1){
				sprintf(temp, "mov edx, %d\n", a_count);
				buf_write(&buf_n, temp, buffer);
				buf_write(&buf_n, "add [ecx], edx\n\n", buffer);
			} else {
				buf_write(&buf_n, "mov ebx, 1\n", buffer);
				buf_write(&buf_n, "add [ecx], ebx\n\n", buffer);
			}
			
		} else if (read_buffer[i] == '-'){
			a_count = 1;
			while (read_buffer[++i] == '-'){
				a_count++;
			} 
			i--;

			//buf_write(&buf_n, "mov eax, buf_p\n", buffer);
			///buf_write(&buf_n, "mov ecx, buf\n", buffer);
			//buf_write(&buf_n, "add ecx, [buf_p]\n", buffer);
			//buf_write(&buf_n, "mov ebx, 1\n", buffer);
			if (a_count > 1){
				sprintf(temp, "mov edx, %d\n", a_count);
				buf_write(&buf_n, temp, buffer);
				buf_write(&buf_n, "sub [ecx], edx\n\n", buffer);
			} else {
				buf_write(&buf_n, "mov ebx, 1\n", buffer);
				buf_write(&buf_n, "sub [ecx], ebx\n\n", buffer);
			}
		} else if (read_buffer[i] == '.'){

			buf_write(&buf_n, "mov eax, 4\n", buffer);
			buf_write(&buf_n, "mov ebx, 1\n", buffer);
			buf_write(&buf_n, "mov edx, 1\n", buffer);
			buf_write(&buf_n, "int 0x80\n", buffer);
			buf_write(&buf_n, "mov eax, buf_p\n", buffer);
			buf_write(&buf_n, "mov ecx, buf\n", buffer);
			buf_write(&buf_n, "add ecx, [buf_p]\n\n", buffer);
		} else if (read_buffer[i] == '>'){
			//buf_write(&buf_n, "mov ecx, buf_p\n", buffer);
			buf_write(&buf_n, "mov ebx, 4\n", buffer);
			buf_write(&buf_n, "add [buf_p], ebx\n", buffer);
			buf_write(&buf_n, "mov ecx, buf\n", buffer);
			buf_write(&buf_n, "add ecx, [buf_p]\n\n", buffer);
		} else if (read_buffer[i] == '<'){
			//buf_write(&buf_n, "mov ecx, buf_p\n", buffer);
			buf_write(&buf_n, "mov ebx, 4\n", buffer);
			buf_write(&buf_n, "sub [buf_p], ebx\n", buffer);
			buf_write(&buf_n, "mov ecx, buf\n", buffer);
			buf_write(&buf_n, "add ecx, [buf_p]\n\n", buffer);
		} else if (read_buffer[i] == '['){
			sprintf(temp, "ll%ds%d:\n", label_l, labels_d[label_l]);
			labels[label_l]++;
			label_l++;
			buf_write(&buf_n, temp, buffer);
		} else if (read_buffer[i] == ']'){
			// buf_write(&buf_n, "mov eax, buf_p\n", buffer);
			// buf_write(&buf_n, "mov ecx, buf\n", buffer);
			// buf_write(&buf_n, "add ecx, [eax]\n", buffer);
			buf_write(&buf_n, "mov edx, [ecx]\n", buffer);
			buf_write(&buf_n, "cmp edx, 0\n", buffer);
			sprintf(temp, "jnz ll%ds%d\n", label_l-1, labels_d[label_l-1]);
			label_l--;
			labels[label_l]--;
			if (labels[label_l] == 0){
				labels_d[label_l]++;
			}
			buf_write(&buf_n, temp, buffer);
		}
	}

	create_exe(buf_n, buffer, argv[1]);

	return 0;
}