# bf-compiler
A C program that converts BF instructions to x86 assembly and then compiles into an executable using nasm and ld.

Will work with basic programs, but there is still some bugs with looping.
## Compiling
Simply compile the C file with the following command:  
```
gcc -o bfc bfc.c
```
## Running
Simply compile your program into an executable with the following command (needs .bf extention):
```
./bfc program.bf
```
Output assembly file:
```
./bfc -S program.bf
```
Output object file:
```
./bfc -O program.bf
```
