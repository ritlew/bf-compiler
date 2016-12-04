#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {

} commands;

void buf_write(int* buf_n, char * str, char ** buffer){
	buffer[*buf_n] = (char *)malloc(strlen(str));
	strcpy(buffer[(*buf_n)++], str);
}

int main(int argc, char* argv[]){
	if (argc < 2){
		printf("No input specified\n");
	}
	int i;
	char * file[10000];
	char * beg = "section	.text\n\
\tglobal _start\n\
\n\
section    .data\n\
\n\
\tbuf times 30000 dd 0\n\
\tbuf_p dd 0\n";

	char * buffer[30000];
	int buf_n = 0;
	int buf_p = 0;
	int labels[500] = {0};
	int labels_d[500] = {0};
	int label_l = 0;
	char read_buffer[30001];

	FILE* read_f;
	read_f = fopen(argv[1], "r");

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




	FILE* f;
	f = fopen("prog.asm", "w");

	fwrite(beg, strlen(beg), 1, f);
	fwrite("_start:\n", strlen("_start:\n"), 1, f);
	for (i = 0; i < buf_n; i++){
		sprintf(temp, "\t%s", buffer[i]);
		fwrite(temp, strlen(temp), 1, f);
	}
	char * end = "\tmov eax, 1\n\tmov ebx, 0\n\tint 0x80";
	fwrite(end, strlen(end), 1, f);


	fclose(f);

	return 0;
}