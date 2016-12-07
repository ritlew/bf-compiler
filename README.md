# bf-compiler
A compiler written in C that converts Brainfuck instructions to x86 assembly and then compiles into an executable using nasm and ld.

The program has 30000 cells, and can only read 30000 total commands (+-.,[]<>) from the file (arbitrary). Cells are 32-bit.

Tested on all programs in examples folder on Ubuntu 16.04.

## Dependencies 
`nasm` is a dependancy for this program. If you get a message like:
```
ld: cannot find hello_world.o: No such file or directory
```
Then you need to install nasm.
```
sudo apt-get install nasm
```

## Compiling
Simply compile the C file with the following command:  
```
gcc -o bfc bfc.c
```
## Running
Simply compile your program into an executable with the following command (needs .b extention):
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
