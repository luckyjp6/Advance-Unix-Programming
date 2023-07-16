all:
	yasm -f elf64 -DYASM -D__x86_64__ -DPIC task3.s -o task3.o
	ld -m elf_x86_64 -o task3 task3.o
	./task3
shm: 
	gcc task2.c -o task2
	./task2