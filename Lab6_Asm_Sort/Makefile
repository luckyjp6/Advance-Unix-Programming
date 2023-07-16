all:
	yasm -f elf64 -DYASM -D__x86_64__ -DPIC lab6.asm -o lab6.o
	ld -m elf_x86_64 -o lab6 lab6.o
	./lab6