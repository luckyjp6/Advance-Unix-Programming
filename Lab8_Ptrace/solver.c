#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t child;
int child_stat;
struct user_regs_struct regs;

unsigned long long magic;

void err_quit(char *msg) {
    perror(msg);
    exit(-1);
}

void do_next() {
    if (ptrace(PTRACE_CONT, child, 0, 0) < 0) err_quit("ptrace cont");
    if (waitpid(child, &child_stat, 0) < 0) err_quit("waitpid");
    if (!WIFSTOPPED(child_stat)) err_quit("child not stop");;
}

void read_args() {
    if (ptrace(PTRACE_GETREGS, child, 0, &regs) < 0) err_quit("ptrace get regs");
}

void set_magic(int n) {
    magic = 0;
    for (int i = 0; i < 8; i++) {
        magic = magic << 8;
        magic += ((n >> i) & 0x1) + 0x30;
    }
}

int main(int argc, char **argv) {

    unsigned long long int reset_rip;
    unsigned long long int magic_addr;

    if (argc < 2) {
        printf("Usage: ./solver <sample exe>");
        return 0;
    }

    if((child = fork()) < 0) err_quit("fork");
    if (child == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) err_quit("ptrace traceme");
        execvp(argv[1], argv+1);
        err_quit("execvp");
    }
    
    if (waitpid(child, &child_stat, 0) < 0) err_quit("waitpid");
    if (ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL) < 0) err_quit("ptrace set options");
    if (!WIFSTOPPED(child_stat)) err_quit("child not stop");
    
    /* start child*/
    do_next();

    /* get magic address */
    if (ptrace(PTRACE_GETREGS, child, 0, &regs) < 0) err_quit("ptrace get regs");
    unsigned long int tmp_codes;
    tmp_codes = ptrace(PTRACE_PEEKTEXT, child, regs.rip+17, 0);
    if (ptrace(PTRACE_POKETEXT, child, regs.rip+17, (tmp_codes & 0xffffffffffffff00) | 0xcc) < 0) err_quit("ptrace poketext");
    do_next();
    read_args();
    magic_addr = regs.rax;
    regs.rip -= 1;
    if (ptrace(PTRACE_SETREGS, child, 0, &regs) < 0) err_quit("ptrace set regs");
    if (ptrace(PTRACE_POKETEXT, child, regs.rip, tmp_codes) < 0) err_quit("ptrace poketext");

    do_next();
    do_next();

    /* recored rip for reset */
    if (ptrace(PTRACE_GETREGS, child, 0, &regs) < 0) err_quit("ptrace get regs");
    reset_rip = regs.rip;

    regs.rip = reset_rip;    
    if (ptrace(PTRACE_SETREGS, child, 0, &regs) < 0) err_quit("ptrace set regs");
    
    /* out of if-else */
    for (int m = 0; m < 512; m++) {
        set_magic(m);
        if (ptrace(PTRACE_POKETEXT, child, magic_addr, magic) < 0) err_quit("ptrace poketext");
        if (m > 255) {
            if (ptrace(PTRACE_POKETEXT, child, magic_addr+8, 0x31) < 0) err_quit("ptrace poketext");
        } else {
            if (ptrace(PTRACE_POKETEXT, child, magic_addr+8, 0x30) < 0) err_quit("ptrace poketext");
        }

        do_next();
        do_next();
        
        read_args();
        if (regs.rax == 0) break; // get Bingo, exit

        regs.rip = reset_rip;    
        if (ptrace(PTRACE_SETREGS, child, 0, &regs) < 0) err_quit("ptrace set regs");
    }
    
    /* print "Magic evaluated" */
    do_next();

    perror("done");


    return 0;
}