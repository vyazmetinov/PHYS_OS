#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(argv[1]);
    if(!dir) {
        printf("Can't open directory %s", argv[1]);
        return -1;
    }
     while ( (entry = readdir(dir)) != NULL) {
        printf("%llu - %s [%d] %d\n",
        entry->d_ino, entry->d_name, entry->d_type, entry->d_reclen);
    }
    closedir(dir);
    return 0;
}

