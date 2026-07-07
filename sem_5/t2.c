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
        int contain[3] = {0, 0, 0};
        for (int i = 0; i < st.st_size; i++) {
            if (text[i] == '\n') {
                contain[0]++;
            }
            else if (text[i] == ' ' || text[i] == '\n') {
                contain[1]++;
            }
            else {
                contain[2]++;
            }
        }
        printf("Количество строк %d, Количество слов %d, Количество символов %d", contain[0], contain[1], contain[2]);
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