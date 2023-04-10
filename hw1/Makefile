all: sandbox.so
	./launcher ./sandbox.so config.txt echo "123"
sandbox.so: sandbox.c
	gcc -o sandbox.so -shared -fPIC sandbox.c -ldl
test:
	gcc sandbox.c -o sandbox -g
	./sandbox
pwn:
	. ~/pwntools/bin/activate