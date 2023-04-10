#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>

#if defined(__LP64__)
#define ElfW(type) Elf64_ ## type
#else
#define ElfW(type) Elf32_ ## type
#endif

void errquit(char *msg) {
    perror(msg);
    exit(0);
}

void parse_elf(const char* elf_file) {
    ElfW(Ehdr) header;

    int fd = open(elf_file, O_RDONLY);
    if (fd < 0) errquit("can't open elf file");

    // read elf header
    read(fd, &header, sizeof(header));

    // check valid
    if (memcmp(header.e_ident, ELFMAG, SELFMAG) != 0) errquit("header invalid");

    // get section header
    int section_hdr_off = header.e_shoff;
    int section_hdr_len = header.e_shentsize;
    int section_hdr_num = header.e_shnum;
    Elf64_Shdr shdr[200];
    char name_table[20000];
    if (lseek(fd, section_hdr_off, SEEK_SET) != section_hdr_off) errquit("section hdr seek");
    // printf("section num: %d\n", section_hdr_num);
    for (int i = 0; i < section_hdr_num; i++) {
        if (read(fd, &shdr[i], sizeof(Elf64_Shdr)) < 0) errquit("section hdr read");
    }
    // get section name table
    int name_idx = header.e_shstrndx;
    if (lseek(fd, shdr[name_idx].sh_offset, SEEK_SET) < 0) errquit("section name table seek");
    if (read(fd, name_table, shdr[name_idx].sh_size) < 0) errquit("section name table read");

    for (int i = 0; i < section_hdr_num; i++) {
        printf("%s\t", &name_table[shdr[i].sh_name]);
        // switch(shdr[i].sh_type) {
        //     case 0 : printf("SHT_NULL"); break;
        //     case 1 : printf("SHT_PROGBITS"); break;
        //     case 2 : printf("SHT_SYMTAB"); break;
        //     case 3 : printf("SHT_STRTAB"); break;
        //     case 4 : printf("SHT_RELA"); break;
        //     case 5 : printf("SHT_HASH"); break;
        //     case 6 : printf("SHT_DYNAMIC"); break;
        //     case 7 : printf("SHT_NOTE"); break;
        //     case 8 : printf("SHT_NOBITS"); break;
        //     case 9 : printf("SHT_REL"); break;
        //     case 10 : printf("SHT_SHLIB"); break;
        //     case 11 : printf("SHT_DYNSYM"); break;
        //     case 14 : printf("SHT_INIT_ARRAY"); break;
        //     case 15 : printf("SHT_FINI_ARRAY"); break;
        //     case 0x70000000 : printf("SHT_LOPROC"); break;
        //     case 0x7fffffff : printf("SHT_HIPROC"); break;
        //     case 0x80000000 : printf("SHT_LOUSER"); break;
        //     case 0xffffffff : printf("SHT_HIUSER"); break;
        //     case 0x6ffffff6 : printf("SHT_GNU_HASH"); break;
        //     case 0x6fffffff : printf("SHT_GNU_versym"); break;
        //     case 0x6ffffffe : printf("SHT_GNU_verneed"); break;
        // }
        printf("\n");
    }
    
    close(fd);
    return;
}

char* get_path(char *instruction) {
    int fd = open("/proc/self/maps", O_RDONLY);
    if (fd < 0) errquit("/proc/self/maps open");
    char buf[20000], *record, *cnt = buf;
    read(fd, buf, sizeof(buf));
    while ((record = strtok_r(cnt, "\n\r", &cnt)) != NULL) {
        if (strstr(buf, instruction) == NULL) continue;
        // 55e2caf7c000-55e2caf7e000 r--p 00000000 08:20 1663 /usr/bin/cat
        printf("%s\n", buf);
        strtok_r(record, ":", &record); // 08:20
        strtok_r(record, " ", &record); // 20
        strtok_r(record, " ", &record); // 1663
        return strtok_r(record, " ", &record); // /usr/bin/cat
    }
    printf("can't get instruction path\n");
    exit(-1);
}

int __libc_start_main(int (*main) (int, char * *, char * *), int argc, char * * argv, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end)) {
// int main() {
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) printf("%d %s\n", i, argv[i]);
    char *path = get_path(argv[0]);
    printf("path: %s\n", path);
    // parse_elf(path);
    // void* handle = dlopen(path, RTLD_LAZY);
    void* handle = dlopen("/lib/x86_64-linux-gnu/libc.so.6", RTLD_LAZY);
    if (!handle) errquit(dlerror());
    void (*real_start)() = dlsym(handle, "__libc_start_main");
    if (!real_start) {
        dlclose(handle);
        errquit("can't get real __libc_start_main");
    }
    real_start(main, argc, argv, init, fini, rtld_fini, stack_end);
    return 0;
}