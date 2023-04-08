#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

int get = 0;
int satisify;

void read_dir(char *name, char *want) {
    DIR *dir = opendir(name);
    if (dir == NULL) {
        cerr << "fail to get directory" << endl;
        return;
    } else {
        cerr << "successfully open dir: " << name << endl;
    }

    // for (int i = 0; i < 2; i++) readdir(dir);
    while(1) {
        struct dirent *f;
        f = readdir(dir);
        if (f == NULL) break;
        if (strcmp(f->d_name, "..")==0 || strcmp(f->d_name, ".")==0) {
            // cerr << "######################" << endl;
            continue;
        }
        char pwd[2000];
        sprintf(pwd, "%s/%s", name, f->d_name);
        cerr << "get file name: " << f->d_name << endl;
        
        if (f->d_type == DT_DIR) {
            read_dir(pwd, want); 
            continue;
        }
        // if (f->d_type != DT_REG) continue;
        // struct stat buf[10000];
        // lstat(pwd, buf);
        int fd = open(pwd, O_RDONLY);
        if (fd < 0) continue;

        char content[10000];
        read(fd, content, 1000);
        char* c = strstr(content, want);
        if (c != NULL) cout << pwd << endl;
        close(fd);
        cerr << "does not find magic numer" << endl;
    }
    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Usage: /solver /path/to/the/files/directory magic-number" << endl;
        return 0;
    }
    satisify = strlen(argv[2]);
    read_dir(argv[1], argv[2]);
    return 0;
}