#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


void my_cat(char path_from[1024], char path_to[1024]) {
    int fd_from = open(path_from, O_RDONLY);
    if (fd_from < 0) { perror("fileopen"); exit(1); }
    int fd_to = open(path_to, O_WRONLY);
    if (fd_to < 0) {
        char cmd[1024];
        sprintf(cmd, "touch %s ", path_to);
        system(cmd);
        fd_to = open(path_to, O_RDONLY);
        if (fd_to < 0) { perror("fileopen"); exit(1); }
    }
    struct stat st;
    if (lstat(path_from, &st) == -1) { perror("lstat"); exit(1); }
    if (S_ISREG(st.st_mode)) {
        char text[st.st_size + 1];
        read(fd_from, text, st.st_size);
        write(fd_to, text, st.st_size);
    }
    close(fd_from);
    close(fd_to);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
    }
    my_cat(argv[1], argv[2]);
    return 0;
}