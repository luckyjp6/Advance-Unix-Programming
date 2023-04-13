#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if defined(__LP64__)
#define ElfW(type) Elf64_ ## type
#else
#define ElfW(type) Elf32_ ## type
#endif

void errquit(char *msg) {
    perror(msg);
    exit(0);
}

void print_Elf64_Sym(Elf64_Sym a) {
    printf("name: %x, bind: %d, type: %d, other: %d, section: %d, value: %lx, size: %ld\n", 
                a.st_name, ELF64_ST_BIND(a.st_info), ELF64_ST_TYPE(a.st_info), a.st_other,
                a.st_shndx, a.st_value, a.st_size
                );
}
void print_section_type(Elf64_Shdr a) {
    switch(a.sh_type) {
        case 0 : printf("SHT_NULL\t"); break;
        case 1 : printf("SHT_PROGBITS"); break;
        case 2 : printf("SHT_SYMTAB\t"); break;
        case 3 : printf("SHT_STRTAB\t"); break;
        case 4 : printf("SHT_RELA\t"); break;
        case 5 : printf("SHT_HASH\t"); break;
        case 6 : printf("SHT_DYNAMIC\t"); break;
        case 7 : printf("SHT_NOTE\t"); break;
        case 8 : printf("SHT_NOBITS\t"); break;
        case 9 : printf("SHT_REL\t"); break;
        case 10 : printf("SHT_SHLIB\t"); break;
        case 11 : printf("SHT_DYNSYM\t"); break;
        case 14 : printf("SHT_INIT_ARRAY"); break;
        case 15 : printf("SHT_FINI_ARRAY"); break;
        case 0x70000000 : printf("SHT_LOPROC"); break;
        case 0x7fffffff : printf("SHT_HIPROC"); break;
        case 0x80000000 : printf("SHT_LOUSER"); break;
        case 0xffffffff : printf("SHT_HIUSER"); break;
        case 0x6ffffff6 : printf("SHT_GNU_HASH"); break;
        case 0x6fffffff : printf("SHT_GNU_versym"); break;
        case 0x6ffffffe : printf("SHT_GNU_verneed"); break;
    }
    printf("\n");
}

void print_log(char *msg, size_t len) {
    int logger_fd;
    char* logger_fd_char = getenv("LOGGER_FD");
    sscanf(logger_fd_char, "%d", &logger_fd);

    write(logger_fd, msg, len);
    return;
}

int config(const char *wanted, const char* func_name) {
    // get config file name
    char* config_name = getenv("SANDBOX_CONFIG");

    // open config
    int fd = open(config_name, O_RDONLY);
    char data[100000];
    read(fd, data, sizeof(data));

    // prepare start & end
    char begin[50]; sprintf(begin, "BEGIN %s-blacklist", func_name);
    char end[50]; sprintf(end, "END %s-blacklist", func_name);
    
    // parse lines
    int start = 0;
    char *d = data;
    while (d != NULL) {
        char* now = strtok_r(d, "\n", &d);
        if (strcmp(now, begin) == 0) start = 1;
        if (strcmp(now, end) == 0) break;
        if (!start) continue;

        if (strcmp(func_name, "open") == 0) {
            if (strcmp(now, wanted) == 0) return -1;
        }
        else if (strcmp(func_name, "read") == 0){
            if (strstr(wanted, now)) {
                printf("here");
                return -1;
            }
        }
    }

    return 1;
}

int my_open(const char *pathname, int flags, mode_t mode) {
    // printf("my open\n");

    // set mode
    if (!((flags & O_CREAT) | (flags & O_TRUNC) | (flags & __O_TMPFILE))) mode = 0;
    
    int result;
    if (config(pathname, "open") > 0) {
        result = open(pathname, flags, mode);
    }else {
        result = -1;
        errno = EACCES;
    }
    
    // logger msg
    char logger_msg[100];
    sprintf(logger_msg, "[logger] open(\"%s\", %d, %d) = %d\n", pathname, flags, mode, result);
    print_log(logger_msg, strlen(logger_msg));

    return result;
}
ssize_t my_read(int fd, void *buf, size_t count) {
    int result = read(fd, buf, count);
    // get pid
    int pid = getpid();

    // get file name
    // char look_up_name[50];
    // sprintf(look_up_name, "/proc/self/fd/%d", fd);
    // int look_up_fd = open(look_up_name, O_RDONLY);
    // char get_name[50];
    // read(look_up_fd, get_name, sizeof(get_name));
    // get_name[strlen(get_name)-1] = 0;
    // close(look_up_fd);

    // char my_file_name[70];
    // sprintf(my_file_name, "./%d-%d-read_%s.log", pid, fd, get_name);
    // int my_log = open(my_file_name, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    // if (my_log < 0) errquit("my log open");

    // check
    if (result > 0) {
        char log_name[50];
        sprintf(log_name, "./%d-%d-read.log", pid, fd);
        int log = open(log_name, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);

        // get now position
        char tmp[100000];
        int now_off = lseek(fd, 0, SEEK_CUR);

        // get current file content
        lseek(log, -now_off, SEEK_END);
        read(log, tmp, now_off);
        strcpy(tmp, buf);

        if (config(tmp, "read") > 0) {
            lseek(log, 0, SEEK_END);
            write(log, buf, result);
        }else {
            close(fd);
            result = -1;
            errno = EIO;
        }
        close(log);
    }

    // logger message
    char logger[100];
    sprintf(logger, "[logger] read(%d, %p, %ld) = %d\n", fd, buf, count, result);
    print_log(logger, strlen(logger));

    return result;
}
ssize_t my_write(int fd, void *buf, size_t count) {
    // get pid
    int pid = getpid();

    // get file name
    char file_name[50];
    sprintf(file_name, "./%d-%d-write.log", pid, fd);
    int log = open(file_name, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    if (log < 0) errquit("log open");
    if (write(log, buf, count) < 0) errquit("write_log write");
    close(log);
    int result = write(fd, buf, count);

    // logger message
    char logger[100];
    sprintf(logger, "[logger] write(%d, %p, %ld) = %d\n", fd, buf, count, result);
    print_log(logger, strlen(logger));

    return result;
}
int my_conn(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    static struct sockaddr_in addr_in;
    memcpy(&addr_in, addr, addrlen);
    printf("%s:%d", addr_in.sin_addr, addr_in.sin_port);

    return connect(sockfd, addr, addrlen);
}
int my_getaddr (const char *restrict node, const char *restrict service, const struct addrinfo *restrict hints, struct addrinfo **restrict res){
    printf("my_getaddr\n");
    return getaddr(node, service, hints, res);
}

void get_write_previlage(long int addr) {
    void *A = (void*)addr;
    uintptr_t ali = (uintptr_t)A;
    ali = ali & 0xfffffffff000;
    void *ali_ptr = (void *)ali;
    // printf("expected: from %p to %p\n", addr, addr+0x1000);
    // printf("actually requested: from %p ot %p\n", ali_ptr, ali_ptr + 0x1000);
    if (mprotect(ali_ptr, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) errquit("mprotect failed");
}
void replace(unsigned long int addr, char* func) {
    get_write_previlage(addr);
    void *handle = dlopen("./sandbox.so", RTLD_LAZY);
    if (!handle) errquit(dlerror());
    void (*my_func)() = dlsym(handle, func);
    if (!my_func) { dlclose(handle); errquit("cant't get my_func "); }
    memcpy((void *)addr, &my_func, 8);
}

void parse_elf(const char* elf_file, long int start_addr) {
    ElfW(Ehdr) header;

    int fd = open(elf_file, O_RDONLY);
    if (fd < 0) errquit("can't open elf file");

    // read elf header
    read(fd, &header, sizeof(header));

    // check valid
    if (memcmp(header.e_ident, ELFMAG, SELFMAG) != 0) errquit("header invalid");

    // get section header
    int section_hdr_off = header.e_shoff;
    int section_hdr_num = header.e_shnum;
    Elf64_Shdr shdr[1000];
    char name_table[20000];
    if (lseek(fd, section_hdr_off, SEEK_SET) != section_hdr_off) errquit("section hdr seek");
    if (read(fd, &shdr, section_hdr_num*sizeof(Elf64_Shdr)) < 0) errquit("section hdr read");
    
    // get section name table
    int name_idx = header.e_shstrndx;
    if (lseek(fd, shdr[name_idx].sh_offset, SEEK_SET) < 0) errquit("section name table seek");
    if (read(fd, name_table, shdr[name_idx].sh_size) < 0) errquit("section name table read");
    
    // for (int i = 0; i < section_hdr_num; i++) { printf("%s\t", &name_table[shdr[i].sh_name]); print_section_type(shdr[i]);}

    // get section idx
    int rela_plt_idx, sym_table_idx, str_table_idx, plt_got_idx;
    for (int i = 0; i < section_hdr_num; i++) {
        if (strcmp(&name_table[shdr[i].sh_name], ".rela.plt") == 0) rela_plt_idx = i;
        if (strcmp(&name_table[shdr[i].sh_name], ".dynsym") == 0) sym_table_idx = i;
        if (strcmp(&name_table[shdr[i].sh_name], ".dynstr") == 0) str_table_idx = i;    
    }
    if (rela_plt_idx < 0) errquit(".rela.plt not found");
    if (sym_table_idx < 0) errquit(".dynsym not found");
    if (str_table_idx < 0) errquit(".dynstr not found");

    // get section .dynsym
    Elf64_Sym sym_name[1000];
    if (lseek(fd, shdr[sym_table_idx].sh_offset, SEEK_SET) < 0) errquit(".dynsym seek");
    if (read(fd, &sym_name, shdr[sym_table_idx].sh_size) < 0) errquit(".dynsym read");

    uint16_t name_off = sym_name[0].st_shndx;
    // get section .dynstr
    if (lseek(fd, shdr[str_table_idx].sh_offset+name_off, SEEK_SET) < 0) errquit(".dynstr seek");
    char names[20000];
    read(fd, names, sizeof(names));
    
    // get section .rela.plt
    const int record_num = shdr[rela_plt_idx].sh_size/sizeof(Elf64_Rela);
    // printf("rela plt num: %d\n", record_num);
    Elf64_Rela record[record_num];
    if (lseek(fd, shdr[rela_plt_idx].sh_offset, SEEK_SET) < 0) errquit(".rela.plt seek");
    if (read(fd, &record, shdr[rela_plt_idx].sh_size) < 0) errquit(".rela.plt read");
    
    int open_idx, read_idx, write_idx, conn_idx, getaddr_idx, sys_idx;
    for (int i = 0; i < record_num; i++) {
        if (strcmp("open", names+sym_name[ELF64_R_SYM(record[i].r_info)].st_name) == 0) open_idx = i;
        if (strcmp("read", names+sym_name[ELF64_R_SYM(record[i].r_info)].st_name) == 0) read_idx = i;
        if (strcmp("write", names+sym_name[ELF64_R_SYM(record[i].r_info)].st_name) == 0) write_idx = i;
        if (strcmp("connect", names+sym_name[ELF64_R_SYM(record[i].r_info)].st_name) == 0) conn_idx = i;
        if (strcmp("getaddrinfo", names+sym_name[ELF64_R_SYM(record[i].r_info)].st_name) == 0) getaddr_idx = i;
        if (strcmp("system", names+sym_name[ELF64_R_SYM(record[i].r_info)].st_name) == 0) sys_idx = i;
        // printf("offset: %lx, sym: %lx, type:%lx, addend: %lx, name: %s\n", 
        //     record[i].r_offset, ELF64_R_SYM(record[i].r_info), ELF64_R_TYPE(record[i].r_info),
        //     record[i].r_addend, names+sym_name[ELF64_R_SYM(record[i].r_info)].st_name);
    }

    if (open_idx > 0) replace(start_addr+record[open_idx].r_offset, "my_open");
    if (read_idx > 0) replace(start_addr+record[read_idx].r_offset, "my_read");
    if (write_idx > 0) replace(start_addr+record[write_idx].r_offset, "my_write");
    if (conn_idx > 0) replace(start_addr+record[conn_idx].r_offset, "my_conn");
    if (getaddr_idx > 0) replace(start_addr+record[getaddr_idx].r_offset, "my_getaddr");
    if (sys_idx > 0) replace(start_addr+record[sys_idx].r_offset, "my_sys");

    
    close(fd);
    return;
}


char* get_path(char *instruction, long int *start_addr) {
    int fd = open("/proc/self/maps", O_RDONLY);
    if (fd < 0) errquit("/proc/self/maps open");
    char buf[20000], *record, *cnt = buf;
    read(fd, buf, sizeof(buf));
    while ((record = strtok_r(cnt, "\n\r", &cnt)) != NULL) {
        if (strstr(buf, instruction) == NULL) continue;
        // 55e2caf7c000-55e2caf7e000 r--p 00000000 08:20 1663 /usr/bin/cat
        // printf("%s\n", buf);
        long int tt;
        char *tmp = strtok_r(record, "-", &record);
        if (tmp == NULL) errquit("can't get start addr");
        sscanf(tmp, "%lx", &tt); *start_addr = tt;
        strtok_r(record, ":", &record); // 08:20
        strtok_r(record, " ", &record); // 20
        strtok_r(record, " ", &record); // 1663
        return strtok_r(record, " ", &record); // /usr/bin/cat
    }
    printf("can't get instruction path\n");
    exit(-1);
}

int __libc_start_main(int (*main) (int, char * *, char * *), int argc, char * * argv, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end)) {
    // printf("argc: %d\n", argc);
    // for (int i = 0; i < argc; i++) printf("%d %s\n", i, argv[i]);
    long int start_addr;
    char *path = get_path(argv[0], &start_addr);
    // char path[200];
    // if (readlink("/proc/self/exe", path, sizeof(path)) < 0) errquit("readlink");
    
    // printf("path: %s\n", path);
    // printf("start addr: %lx\n", start_addr);
    // handle = dlopen(path, RTLD_LAZY);
    // if (*real_start)() = dlsym(handle, "path");
    // void (*tmp)() = dlsym(handle, "open");
    // if (tmp == NULL) {
    //     dlclose(handle);
    //     errquit("can't get open");
    // }
    // memcpy(my_open, &tmp) 
    
    parse_elf(path, start_addr);

    void *handle = dlopen("/lib/x86_64-linux-gnu/libc.so.6", RTLD_LAZY);
    if (!handle) errquit(dlerror());
    void (*real_start)() = dlsym(handle, "__libc_start_main");
    if (!real_start) {
        dlclose(handle);
        errquit("can't get real __libc_start_main");
    }
    real_start(main, argc, argv, init, fini, rtld_fini, stack_end);
    return 0;
}