#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/wait.h>

int only_asm_h = 0;
int only_object = 0;
typedef struct {
	char * buffer[30000];
	int n;
} asm_h;

void buf_write(asm_h* lines, char * str){
	lines->buffer[lines->n] = (char *)malloc(strlen(str));
	strcpy(lines->buffer[lines->n++], str);
}

void bf_to_asm(asm_h* lines, char* commands){
	int i;

	int labels[500] = {0};
	int labels_d[500] = {0};
	int label_l = 0;

	char temp[80];
	int a_count;
	buf_write(lines, "mov ebx, 1\n");
	buf_write(lines, "mov eax, buf_p\n");
	buf_write(lines, "mov ecx, buf\n");
	buf_write(lines, "add ecx, [buf_p]\n\n");
	for (i = 0; i < strlen(commands); i++){
		if (commands[i] == '+'){
			a_count = 1;
			while (commands[++i] == '+'){
				a_count++;
			} 
			i--;

			//buf_write(lines, "mov eax, buf_p\n");
			///buf_write(lines, "mov ecx, buf\n");
			//buf_write(lines, "add ecx, [buf_p]\n");
			//buf_write(lines, "mov ebx, 1\n");
			if (a_count > 1){
				sprintf(temp, "mov edx, %d\n", a_count);
				buf_write(lines, temp);
				buf_write(lines, "add [ecx], edx\n\n");
			} else {
				buf_write(lines, "mov ebx, 1\n");
				buf_write(lines, "add [ecx], ebx\n\n");
			}
			
		} else if (commands[i] == '-'){
			a_count = 1;
			while (commands[++i] == '-'){
				a_count++;
			} 
			i--;

			//buf_write(lines, "mov eax, buf_p\n");
			///buf_write(lines, "mov ecx, buf\n");
			//buf_write(lines, "add ecx, [buf_p]\n");
			//buf_write(lines, "mov ebx, 1\n");
			if (a_count > 1){
				sprintf(temp, "mov edx, %d\n", a_count);
				buf_write(lines, temp);
				buf_write(lines, "sub [ecx], edx\n\n");
			} else {
				buf_write(lines, "mov ebx, 1\n");
				buf_write(lines, "sub [ecx], ebx\n\n");
			}
		} else if (commands[i] == '.'){

			buf_write(lines, "mov eax, 4\n");
			buf_write(lines, "mov ebx, 1\n");
			buf_write(lines, "mov edx, 1\n");
			buf_write(lines, "int 0x80\n");
			buf_write(lines, "mov eax, buf_p\n");
			buf_write(lines, "mov ecx, buf\n");
			buf_write(lines, "add ecx, [buf_p]\n\n");
		} else if (commands[i] == '>'){
			//buf_write(lines, "mov ecx, buf_p\n");
			buf_write(lines, "mov ebx, 4\n");
			buf_write(lines, "add [buf_p], ebx\n");
			buf_write(lines, "mov ecx, buf\n");
			buf_write(lines, "add ecx, [buf_p]\n\n");
		} else if (commands[i] == '<'){
			//buf_write(lines, "mov ecx, buf_p\n");
			buf_write(lines, "mov ebx, 4\n");
			buf_write(lines, "sub [buf_p], ebx\n");
			buf_write(lines, "mov ecx, buf\n");
			buf_write(lines, "add ecx, [buf_p]\n\n");
		} else if (commands[i] == '['){
			sprintf(temp, "ll%ds%d:\n", label_l, labels_d[label_l]);
			labels[label_l]++;
			label_l++;
			buf_write(lines, temp);
		} else if (commands[i] == ']'){
			// buf_write(lines, "mov eax, buf_p\n");
			// buf_write(lines, "mov ecx, buf\n");
			// buf_write(lines, "add ecx, [eax]\n");
			buf_write(lines, "mov edx, [ecx]\n");
			buf_write(lines, "cmp edx, 0\n");
			sprintf(temp, "jnz ll%ds%d\n", label_l-1, labels_d[label_l-1]);
			label_l--;
			labels[label_l]--;
			if (labels[label_l] == 0){
				labels_d[label_l]++;
			}
			buf_write(lines, temp);
		}
	}
}

char * read_bf(char *filename){
	int i;

	char f1_hold[1000];
	char* commands = (char *)malloc(30000);
	FILE* read_f;
	int total_c;
	int real_ptr;

	sprintf(f1_hold, "%s.bf", filename);
	read_f = fopen(f1_hold, "r");

	total_c = fread(commands, 1, 30000, read_f);
	int r_ptr = 0;
	for (i = 0; i < total_c; i++){
		if (strchr(".+-<>[]", commands[i])){
			commands[r_ptr++] = commands[i];
		}
	}

	fclose(read_f);

	commands[r_ptr] = '\0';

	return commands;
}

void write_asm(asm_h* lines, char * filename){
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

	for (i = 0; i < lines->n; i++){
		sprintf(f1_hold, "\t%s\n", lines->buffer[i]);
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
    
	if (!only_asm_h){
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

int main(int argc, char* argv[]){
	if (argc < 2){
		fprintf(stderr, "No input specified\n");
		return -1;
	}

	int i;
	int opt;

	while ((opt = getopt(argc, argv, "SO")) != -1){
		switch (opt){
			case 'S': only_asm_h = 1; break;
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

	//char * file[10000];

	asm_h* lines = malloc(sizeof(asm_h));
	lines->n = 0;

	char* commands = read_bf(filename);

	bf_to_asm(lines, commands);

	write_asm(lines, filename);
	create_exe(filename);

	return 0;
}