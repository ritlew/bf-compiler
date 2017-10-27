# bf-compiler
A compiler written in C that converts bf instructions to x86 assembly and then compiles into an executable using nasm and ld.

The program has 30000 cells, and can only read 30000 total commands (+-.,[]<>) from the file. Cells are 32-bit.

Tested on all programs in examples folder on Ubuntu 16.04 and Fedora 25. It should work on any linux distrobution. Not supported on Windows.

## Dependencies 
Now using `as` instead of `nasm`! The only dependency is `gcc`.

## Compiling
Simply compile the C file with the following command:  
```
gcc -o bfc bfc.c
```
or
```
make
```

## Running
Simply compile your program into an executable with the following 
command (needs .bf extention):
```
./bfc examples/hello_world.b
```
Output assembly file:
```
./bfc -S examples/hello_world.b
```
Output object file:
```
./bfc -O examples/hello_world.b
```
