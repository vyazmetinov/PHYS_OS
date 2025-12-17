#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


void my_cat(char path[1024]) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("fileopen"); exit(1); }
    struct stat st;
    if (lstat(path, &st) == -1) { perror("lstat"); exit(1); }
    if (S_ISREG(st.st_mode)) {
        char text[st.st_size + 1];
        read(fd, text, st.st_size);
        text[st.st_size] = '\0';
        printf("%s\n", text);
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
    }
    my_cat(argv[1]);
    return 0;
}