#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/wait.h>

int only_asm = 0;
int only_object = 0;

void write_asm(int buf_n, char ** buffer, char * filename){
	char f1_hold[1000];
	int i;
    FILE* f;
    
	char * beg[7] = {
		"section\t.text\n",
		"\tglobal _start\n",
		"\n",
		"section\t.data\n",
		"\tbuf times 30000 dd 0\n",
		"\tbuf_p dd 0\n\n",
		"_start:\n"
	};

	char * end[3] = {
		"\tmov eax, 1\n",
		"\tmov ebx, 0\n",
		"\tint 0x80\n"
	};

	sprintf(f1_hold, "%s.asm", filename);
	f = fopen(f1_hold, "w");

	for (i = 0; i < 7; i++){
		fwrite(beg[i], strlen(beg[i]), 1, f);
	}

	for (i = 0; i < buf_n; i++){
		sprintf(f1_hold, "\t%s", buffer[i]);
		fwrite(f1_hold, strlen(f1_hold), 1, f);
	}

	for (i = 0; i < 3; i++){
		fwrite(end[i], strlen(end[i]), 1, f);
	}
	
	fclose(f);
}

void create_exe(char * filename){
    int pid;
    char f1_hold[1000];
    char f2_hold[1000];
    
	if (!only_asm){
	    pid = fork();
		if (pid){
			wait(NULL);
			sprintf(f1_hold, "%s.asm", filename);
			unlink(f1_hold);
			if (!only_object){
				sprintf(f1_hold, "%s.o", filename);
				unlink(f1_hold);
			}
		} else {
			pid = fork();
		    if (pid){
		    	wait(NULL);
		    	if (!only_object){
		    		sprintf(f1_hold, "%s.o", filename);
			    	char * commands[7] = {"ld", "-m", "elf_i386", "-o", filename, f1_hold, NULL};
					execvp(commands[0], commands);
				} else {
					exit(0);
				}
		    } else {
		    	sprintf(f1_hold, "%s.o", filename);
		    	sprintf(f2_hold, "%s.asm", filename);
		    	char * commands[7] = {"nasm", "-f", "elf", "-o", f1_hold, f2_hold, NULL};
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
		fprintf(stderr, "No input specified\n");
		return -1;
	}

	int i;
	int opt;

	while ((opt = getopt(argc, argv, "SO")) != -1){
		switch (opt){
			case 'S': only_asm = 1; break;
			case 'O': only_object = 1; break;
			default: ;
		}
	}

	char* loc = rindex(argv[argc-1], '.');

	if (loc <= 0){
		fprintf(stderr, "Error parsing filename\n");
	}
	char filename[loc - argv[argc-1]];
	memcpy((void *)filename, (void *)argv[argc-1], loc-argv[argc-1]);

	char * file[10000];
	char * buffer[30000];
	int buf_n = 0;

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

	write_asm(buf_n, buffer, filename);
	create_exe(filename);

	return 0;
}