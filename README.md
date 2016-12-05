# bf-compiler
A compiler written in C that converts BF instructions to x86 assembly and then compiles into an executable using nasm and ld.

Tested on all programs in examples folder on Ubuntu 16.04.

## Dependencies 
`nasm` is a dependancy for this program.
```
sudo apt-get install nasm
```

## Compiling
Simply compile the C file with the following command:  
```
gcc -o bfc bfc.c
```
## Running
Simply compile your program into an executable with the following command (needs .bf extention):
```
./bfc examples/hello_world.bf
```
Output assembly file:
```
./bfc -S examples/hello_world.bf
```
Output object file:
```
./bfc -O examples/hello_world.bf
```
