all: sandbox.so
	./launcher ./sandbox.so config.txt cat hello.txt
sandbox.so: sandbox.c
	gcc -o sandbox.so -shared -fPIC sandbox.c -ldl
test:
	gcc sandbox.c -o sandbox -g
	./sandbox
pwn:
	. ~/pwntools/bin/activate

testc:
	gcc test.c -o test
	./test