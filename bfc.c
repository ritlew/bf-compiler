#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/wait.h>

// global variables
// boolean, output only assembly code
int only_asm = 0;
// boolean, output only object file
int only_object = 0;

// struct for assembly code
// each line of assembly goes into the array
// e.g.
// buffer[20] == "mov ecx, buf"
// buffer[21] == "add ecx, [buf_p]"
typedef struct {
	char * buffer[30000];
	int n;
} asm_h;

// shortcut for writing a line to the struct
void buf_write(asm_h* lines, char * str){
	// make room
	lines->buffer[lines->n] = (char *)malloc(strlen(str));
	// copy the string into the buffer
	strcpy(lines->buffer[lines->n++], str);
}

// this function takes a char* of BF commands and converts it to assembly code
// param lines: pointer to struct for output
// param commands: string of BF command characters
void bf_to_asm(asm_h* lines, char* commands){
	// counter variable
	int i;

	/* the following two variables are for the construction of loop label in assembly */

	// this variable counts how many labels have already been used on the level
	// corresponding to the index
	// this is to stop duplicate labels form being created
	// e.g.
	/*
		[++> //labels_d == {0, 0, 0, 0, ...}
			[+>- // labels == {0, 0, 0, 0, ...}
			] // labels == {0, 1, 0, 0, ...}
		] // labels == {1, 1, 0, 0, ...}
		[...-> //labels_d == {1, 1, 0, 0, ...}
			[...- // labels == {1, 1, 0, 0, ...}
			] // labels == {1, 2, 0, 0, ...}
		] // labels == {2, 2, 0, 0, ...}
	*/
	int labels_d[500] = {0};
	// this variable keeps track of what level the program is on
	// e.g.
	/*
		[++> //label_l == 1
			[+>- // label_l == 2
			] // label_l == 1
		] // label_l == 0
	*/
	int label_l = 0;

	// temporary storage for sprintf
	char temp[80];
	// counter for sequenced + or -
	int a_count;
	/* set up registers for first time */
	// the 30k byte buffer starts at buf
	// the pointer which moves around with '<' and '>' is buf_p

	// buf_p is generally in eax
	buf_write(lines, "mov eax, buf_p");
	// buf is generally in ecx
	buf_write(lines, "mov ecx, buf\n");
	for (i = 0; i < strlen(commands); i++){
		// logic for '+' command
		// this command adds 1 to the current cell
		if (commands[i] == '+'){
			/* optimization to count sequenced '+'s */
			// we already know there is at least one '+'
			a_count = 1;
			// count them until we run into one that is not a '+'
			while (commands[++i] == '+'){
				a_count++;
			} 
			// went 1 too far when counting
			i--;
			// if we have more than one
			if (a_count > 1){
				// put that many in edx
				sprintf(temp, "mov edx, %d", a_count);
				buf_write(lines, temp);
				// add them to the current cell
				buf_write(lines, "add [ecx], edx\n");
			// we only have one '+'
			} else {
				// add only 1 to the current cell
				buf_write(lines, "mov ebx, 1");
				buf_write(lines, "add [ecx], ebx\n");
			}
		// logic for '-' command
		// this command subtracts one from the current cell
		} else if (commands[i] == '-'){
			// refer to comments for '+' command
			a_count = 1;
			while (commands[++i] == '-'){
				a_count++;
			} 
			i--;

			if (a_count > 1){
				sprintf(temp, "mov edx, %d", a_count);
				buf_write(lines, temp);
				// subtract that many from the current cell
				buf_write(lines, "sub [ecx], edx\n");
			} else {
				buf_write(lines, "mov ebx, 1");
				// subtract from the current cell
				buf_write(lines, "sub [ecx], ebx\n");
			}
		// logic for '.' command
		// this command prints out the ascii character for the current cell
		} else if (commands[i] == '.'){
			// put 4 in eax, for sys_write
			buf_write(lines, "mov eax, 4");
			// put 1 in ebx, for stdout
			buf_write(lines, "mov ebx, 1");
			// put 1 in edx, for length 1 character array (1 character)
			buf_write(lines, "mov edx, 1");
			// call sys_write
			// (buf pointer is already in ecx)
			buf_write(lines, "int 0x80");
			// put buf_p back in eax, as that is where its expected to be for other commands
			buf_write(lines, "mov eax, buf_p");
			// put buf back in ecx, as taht is where its expected to be for other commands
			buf_write(lines, "mov ecx, buf");
			// put ecx at the correct index
			buf_write(lines, "add ecx, [buf_p]\n");
		// logic for '>' command
		// this command moves the pointer 1 cell to the right
		} else if (commands[i] == '>'){
			// put 4 in ebx, as cells are 4 bytes wide
			buf_write(lines, "mov ebx, 4");
			// add this to buf_p
			buf_write(lines, "add [buf_p], ebx");
			// reset ecx location
			buf_write(lines, "mov ecx, buf");
			// add the new pointer to ecx
			buf_write(lines, "add ecx, [buf_p]\n");
		// logic for '<' command
		// this command moves the pointer 1 cell to the left
		// refer to comments to the '+' command
		} else if (commands[i] == '<'){
			buf_write(lines, "mov ebx, 4");
			// subtract this from buf_p
			buf_write(lines, "sub [buf_p], ebx");
			buf_write(lines, "mov ecx, buf");
			buf_write(lines, "add ecx, [buf_p]\n");
		// logic for '[' command
		// this command starts a loop 
		} else if (commands[i] == '['){
			// create the label for the loop
			buf_write(lines, "mov edx, [ecx]");
			// compare with 0
			buf_write(lines, "cmp edx, 0");
			sprintf(temp, "jz el%ds%d", label_l, labels_d[label_l]);
			buf_write(lines, temp);
			sprintf(temp, "ll%ds%d:", label_l, labels_d[label_l]);
			// put the label in assembly
			buf_write(lines, temp);
			// manage label variables
			label_l++;
		// logic for ']' command
		// this command marks the end of a loop
		// the current cell will be comapred with 0, and repeat if it is not
		} else if (commands[i] == ']'){
			// put the value at the current cell into edx
			buf_write(lines, "mov edx, [ecx]");
			// compare with 0
			buf_write(lines, "cmp edx, 0");
			// jump to corresponding label if they are not equal
			sprintf(temp, "jnz ll%ds%d", label_l-1, labels_d[label_l-1]);
			// add jump command to assembly
			buf_write(lines, temp);
			// manage label variables
			// go down one level
			label_l--;
			sprintf(temp, "el%ds%d:", label_l, labels_d[label_l]);
			buf_write(lines, temp);
			// one less label on this level in progress
			// label is now done
			// add to completed labels
			labels_d[label_l]++;

		}
	}
}

// this fuction will open a file and read all characters into a buffer
// it will then purge any non-bf commands from the buffer and return it
char * read_bf(char *filepath){
	// counter variable
	int i;

	// array for reading/commands
	char* commands = (char *)malloc(30000);
	// file pointer
	FILE* read_f;
	// total characters read from file
	int total_c;
	// pointer to the end of the real bf commands
	int r_ptr = 0;

	read_f = fopen(filepath, "r");

	// read all characters
	total_c = fread(commands, 1, 30000, read_f);

	// done with file
	fclose(read_f);
	
	// copy the array onto itself, omiting any non-bf commands
	// it should be noted that this process is not neccessary for bf_to_asm to work,
	// but it optimizes sequenced + and -
	for (i = 0; i < total_c; i++){
		// check if the character is a bf command
		if (strchr(".+-<>[]", commands[i])){
			// put it in the array if so
			commands[r_ptr++] = commands[i];
		}
	}

	// put a cap on the array
	commands[r_ptr] = '\0';

	return commands;
}

// this function writes the assembly to a file
void write_asm(asm_h* lines, char * filename){
	// counter variable
	int i;
	// temporary buffer for sprintf
	char f1_hold[1000];
	// file pointer
    FILE* f;
    
    // header for assembly
	char * beg[7] = {
		"section\t.text\n",
		"\tglobal _start\n",
		"\n",
		"section\t.data\n",
		"\tbuf times 30000 dd 0\n",
		"\tbuf_p dd 0\n\n",
		"_start:\n"
	};

	// make sure to always exit on program end
	char * end[3] = {
		"\tmov eax, 1\n",
		"\tmov ebx, 0\n",
		"\tint 0x80\n"
	};

	// file name
	sprintf(f1_hold, "%s.asm", filename);
	// open file for writing
	f = fopen(f1_hold, "w");

	// write header
	for (i = 0; i < sizeof(beg)/sizeof(beg[0]); i++){
		fwrite(beg[i], strlen(beg[i]), 1, f);
	}

	// write assembly for bf commands
	for (i = 0; i < lines->n; i++){
		sprintf(f1_hold, "\t%s\n", lines->buffer[i]);
		fwrite(f1_hold, strlen(f1_hold), 1, f);
	}

	// write sys_exit
	for (i = 0; i < sizeof(end)/sizeof(end[0]); i++){
		fwrite(end[i], strlen(end[i]), 1, f);
	}
	
	// close the file
	fclose(f);
}

// this function creates the final exe, or object file
void create_exe(char * filename){
	// pid for forking
    int pid;
    // buffer for sprintf
	char f1_hold[1000];
	// buffer for sprintf
	char f2_hold[1000];
	// fork, one for controller, one to fork again for compiling
    pid = fork();
    //parent
	if (pid){
		// wait for children
		wait(NULL);
		// delete .asm file
		sprintf(f1_hold, "%s.asm", filename);
		unlink(f1_hold);
		// if they didn't want just the object file
		if (!only_object){
			// delete it
			sprintf(f1_hold, "%s.o", filename);
			unlink(f1_hold);
		}
	// child of first fork
	} else {
		// fork again if they don't just want the object file
		if (!only_object){
			pid = fork();
		}
		// this will always be 0 if they only wanted the obect file
	    if (pid){
	    	// wait for children
	    	wait(NULL);
	    	// if they didn' want just the object file
    	
    		// compile into executable
    		// get filename for object file
    		sprintf(f1_hold, "%s.o", filename);
    		// call ld to link object file
	    	char * commands[7] = {"ld", "-m", "elf_i386", "-o", filename, f1_hold, NULL};
	    	// exec
			execvp(commands[0], commands);
		// create the object file
	    } else {
	    	// object file filename
	    	sprintf(f1_hold, "%s.o", filename);
	    	// assembly file filename
	    	sprintf(f2_hold, "%s.asm", filename);
	    	// commands to call nasm to assemble the assembly
	    	char * commands[7] = {"nasm", "-f", "elf", "-o", f1_hold, f2_hold, NULL};
	    	// exec
			execvp(commands[0], commands);
	    }	
	}
}

// little function to handle the O and S switches
void handle_argv(int argc, char* argv[]){
	// make sure they have at least 2 things on the command line
	if (argc < 2){
		fprintf(stderr, "No input specified\n");
		exit(-1);
	}

	// counter
	int i;
	// var for getopt
	int opt;

	while ((opt = getopt(argc, argv, "SO")) != -1){
		switch (opt){
			// set flag to only create assembly file
			case 'S': only_asm = 1; break;
			// set flag to only create object file
			case 'O': only_object = 1; break;
		}
	}
}

int main(int argc, char* argv[]){
	handle_argv(argc, argv);

	// location of '.' for extension
	char * loc_e = rindex(argv[argc-1], '.');

	// if there was no file extention
	if (loc_e < 0){
		// exit
		fprintf(stderr, "Error parsing filename\n");
		exit(-1);
	}

	// location of last '/' if they gave a file in a different directory
	char * loc_s = rindex(argv[argc-1], '/');
	int len;
	// len is from just after '/' to just before '.'
	if (loc_s > 0){
		len = loc_e - loc_s - 1;
	// len is from beginning to '.''
	} else {
		len = loc_e - argv[argc-1];
	}
	// create filename 
	char filename[len];
	// copy the base filename into the buffer
	memcpy((void *)filename, (void *)argv[argc-1] + strlen(argv[argc-1]) - len - 3, len);

	// create the assembly containter struct
	asm_h* lines = malloc(sizeof(asm_h));
	lines->n = 0;

	// get bf commands
	char* commands = read_bf(argv[argc-1]);

	// convert bf to assembly
	bf_to_asm(lines, commands);

	// write the assembly file
	write_asm(lines, filename);
	// if they wanted object for executable
	if (!only_asm){
		// create_exe
		create_exe(filename);
	}

	// all done
	return 0;
}