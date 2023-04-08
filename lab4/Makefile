all: solver.c
	gcc -g solver.c -o solver
submit: 
	python3 submit.py solver
remote:
	gcc -g remoteguess.c -o remoteguess_test -fno-stack-protector
test: test.asm
	nasm -f elf64 -o test.o test.asm
	ld test.o -o test