#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>


void trim_newline(char *s) {
    size_t i = 0;
    while (s[i] && s[i] != '\r' && s[i] != '\n') i++;
    s[i] = '\0';
}


int is_dir_path(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) return 0;
    return S_ISDIR(st.st_mode);
}



const char abs_path[] = "/Users/ivan/CLionProjects/Phys_OS/";

void copy_files(char from_dir0[1024], char to_dir0[1024]) {
    char listing[1024];
    sprintf(listing, "ls -1 \"%s\" > \"%s%s\"", from_dir0, abs_path, "dir_list.txt");
    system(listing);
    strcat(from_dir0, "/");
    strcat(to_dir0, "/");
    char filename[1024];
    sprintf(filename, "%s%s", abs_path, "dir_list.txt");
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf(":)");
        perror("fopenAAAA");
    }
    char line[1024];
    char file_name[1024];
    while (fgets(line, sizeof(line), fp)) {
        strcpy(file_name, line);
        trim_newline(file_name);
        if (file_name[0] == '\0') continue;

        char from_path[1024];
        char to_path[1024];
        strcpy(from_path, from_dir0);
        strcpy(to_path,   to_dir0);
        strcat(from_path, file_name);
        strcat(to_path,   file_name);


        if (!is_dir_path(from_path)) {
            FILE *from_file = fopen(from_path, "rb");
            FILE *to_file   = fopen(to_path,   "wb");
            if (from_file == NULL || to_file == NULL) {
                perror("Не удалось открыть файл для копирования");
            } else {
                int ch;
                while ((ch = fgetc(from_file)) != EOF) {
                    fputc(ch, to_file);
                }
                fclose(from_file);
                fclose(to_file);
            }
            char gzip_file[1200];
            sprintf(gzip_file, "gzip \"%s\"", to_path);
            system(gzip_file);
        }
    }
        fclose(fp);

    sprintf(filename, "%s%s", abs_path, "dir_list.txt");
    FILE *fp_1 = fopen(filename, "r");
    if (fp_1 == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, sizeof(line), fp_1)) {
        strcpy(file_name, line);
        trim_newline(file_name);
        if (file_name[0] == '\0') continue;

        char from_path[1024];
        char to_path[1024];
        strcpy(from_path, from_dir0);
        strcpy(to_path,   to_dir0);
        strcat(from_path, file_name);
        strcat(to_path,   file_name);

        if (is_dir_path(from_path)) {
            char mkdir_cmd[1200];
            sprintf(mkdir_cmd, "mkdir -p \"%s\"", to_path);
            system(mkdir_cmd);
            printf("%s\n%s\n", from_path, to_path);
            copy_files(from_path, to_path);
        }
    }
    fclose(fp_1);

}
void second_task() {
    printf("second_task\n");
    char from_dir[1024];
    char to_dir[1024];
    if (fgets(from_dir, sizeof(from_dir), stdin) == NULL) return;
    if (fgets(to_dir, sizeof(to_dir), stdin) == NULL) return;
    trim_newline(from_dir);
    trim_newline(to_dir);
    copy_files(from_dir, to_dir);
}

int main(int argc, char *argv[])
{
    copy_files(argv[1], argv[2]);
    return 0;
}