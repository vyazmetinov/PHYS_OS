#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

const char* file_type(mode_t mode) {
    if (S_ISREG(mode))  return "regular file";
    if (S_ISDIR(mode))  return "directory";
    if (S_ISLNK(mode))  return "symbolic link";
    if (S_ISCHR(mode))  return "char device";
    if (S_ISBLK(mode))  return "block device";
    if (S_ISFIFO(mode)) return "FIFO/pipe";
    if (S_ISSOCK(mode)) return "socket";
    return "unknown";
}

int main(int argc, char *argv[]) {
    const char *dirname;
    DIR *dir;
    struct dirent *entry;

    if (argc > 1)
        dirname = argv[1];
    else
        dirname = ".";   // текущая директория

    dir = opendir(dirname);
    if (!dir) {
        perror("opendir");
        exit(1);
    }

    printf("Directory: %s\n", dirname);

    while ((entry = readdir(dir)) != NULL) {
        char path[4096];
        struct stat st;

        // формируем полный путь
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        // lstat: важно для символических ссылок
        if (lstat(path, &st) == -1) {
            perror("lstat");
            continue;
        }

        printf("%-30s  (%s)\n", entry->d_name, file_type(st.st_mode));
    }

    closedir(dir);
    return 0;
}