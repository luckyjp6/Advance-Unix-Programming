#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

int get = 0;
int satisify;

void read_dir(char *dir_path, char *want) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        cerr << "fail to get directory" << endl;
        return;
    } else {
        cerr << "successfully open dir: " << dir_path << endl;
    }

    // traverse all the files
    while(1) {
        struct dirent *f;
        char pwd[2000];
        f = readdir(dir);
        
        // end condition
        if (f == NULL) break;
        // skip "." and ".."
        if (strcmp(f->d_name, "..")==0 || strcmp(f->d_name, ".")==0) continue; 

        // get file path
        sprintf(pwd, "%s/%s", dir_path, f->d_name);
        
        // for directory
        if (f->d_type == DT_DIR) {
            read_dir(pwd, want); 
            continue;
        }
        
        // for files
        int fd = open(pwd, O_RDONLY);
        char *c, content[10000];
        if (fd < 0) continue;
        read(fd, content, 10000);
        c = strstr(content, want);
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