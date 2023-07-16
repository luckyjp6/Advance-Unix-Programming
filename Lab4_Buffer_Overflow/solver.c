#include <stdio.h>

typedef int (*printf_ptr_t)(const char *format, ...);

void solver(printf_ptr_t fptr) {
    char msg[16] = "hello world!";
    long long int base = (long long int)(&msg);
    fptr("%lx\n%lx\n%lx\n", *(long long int*)(base+0x18), *(long long int*)(base+0x20), *(long long int*)(base+0x28)); // canary rbp ra

    // canary = *(long long int*)((long long int)(&msg)+0x018);
    // rbp = *(long long int*)((long long int)(&msg)+0x020);
    // ra = *(long long int*)((long long int)(&msg)+0x028);
    // fptr("canary: %llx\n", canary); // canary
    // fptr("rbp: %llx\n", rbp); // rbp
    // fptr("ra: %llx\n", ra); // ra

    fflush(stdout);
    return;
}

int main() {
	char fmt[16] = "** main = %p\n";
	printf(fmt, main);
	solver(printf);
	return 0;
}