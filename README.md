# bf-compiler
A C program that converts BF instructions to x86 assembly and then compiles into an executable using nasm and ld.

Will work with basic Hello World program, but there is still some bug with loops.
## Compiling
Simply compile the program like this:  
```
gcc -o bfc bfc.c
```
## Running
Simple compile your program like (needs .bf extention):
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
