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
#include <sys/sem.h>


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
void first_task() {
    printf("first_task\n");
    char comm_path[1024];
    sprintf(comm_path, "%s%s", abs_path, "comm.txt");
    FILE *fp = fopen(comm_path, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    char line[1024];
    int delay = 0;
    char str[1024];
    while (fscanf(fp, "%d%s", &delay, str) != EOF) {
        printf("%s", str);
        system(str);
        sleep(delay);
    }
}



int is_directory(char name[1024]) {
    int i = -1;
    while (name[++i] != '\0') {
        if (name[i] == '.') {
            return 0;
        }
    }
    return 1;
}
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

int PI() {
        const char *source_path = "/Users/ivan/CLionProjects/Phys_OS/main.c";

    /* Генерируем ключ по файлу и символу 'C'.
       ftok() использует идентификатор файла (inode/устройство) и младшие 8бит project id,
       чтобы вернуть уникальный ключ для shmget()*/
    key_t key = ftok(source_path, 'C');
    if (key == (key_t)-1) {
        perror("ftok");
        return EXIT_FAILURE;
    }

    /* Определяем размер исходного файла (через stat()) и прибавляем один байт
       под завершающий '\0'. */
    struct stat st;
    if (stat(source_path, &st) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }
    size_t size = (size_t)st.st_size + 1;

    /* Создаём/открываем сегмент разделяемой памяти нужного размера (0666— права rw‑rw‑rw‑).
       Если сегмент уже существует, shmget() просто вернёт его shmid. */
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return EXIT_FAILURE;
    }

    /* Подключаем сегмент к адресному пространству процесса. */
    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("shmat");
        return EXIT_FAILURE;
    }

    /* Открываем собственный файл и читаем его в shared memory. */
    FILE *f = fopen(source_path, "rb");
    if (!f) {
        perror("fopen");
        shmdt(shmaddr);
        return EXIT_FAILURE;
    }

    fread(shmaddr, 1, st.st_size, f);
    ((char *)shmaddr)[st.st_size] = '\0';  /* нуль‑терминатор */
    fclose(f);

    printf("Скопировано %ld байт в shared memory (shmid = %d)\\n",
           (long)st.st_size, shmid);

    /* Отсоединяем сегмент.  Функция shmdt() удаляет отображение сегмента из
       адресного пространства процесса, но сам сегмент остаётся в ядре и
       может быть повторно подключён [oai_citation:1‡csl.mtu.edu](https://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/shmdt.html#:~:text=System%20call%20shmdt,a%20shared%20memory%2C%20use%20shmctl). */
    shmdt(shmaddr);

    shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return EXIT_FAILURE;
    }

    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) {
        perror("shmat");
        return EXIT_FAILURE;
    }

    /* Выводим строку, записанную в разделяемую память. */
    printf("Содержимое разделяемой памяти:\\n\\n%s\\n", data);

    /* Отсоединяем сегмент. Он остаётся в ядре до тех пор, пока не будет
       удалён через shmctl() и пока к нему подключены процессы [oai_citation:2‡csl.mtu.edu](https://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/shmdt.html#:~:text=System%20call%20shmdt,a%20shared%20memory%2C%20use%20shmctl). */
    shmdt(data);

    /* Удаляем сегмент, чтобы не оставлять «мусор» в системе.
       Если другим программам ещё нужен доступ, этот вызов можно не делать. */
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}

int fourth_task() {
    int key = ftok("/Users/ivan/CLionProjects/Phys_OS/main.c", 0);
    if (key == (key_t)-1) {
        perror("ftok");
        return EXIT_FAILURE;
    }
    int array[] = {0};
    int shmid = shmget(key, sizeof(array) + 1, IPC_CREAT|0666);
    if (shmid == -1) {
        perror("shmget1");
        return EXIT_FAILURE;
    }

    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("shmat");
        return EXIT_FAILURE;
    }
    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) {
        perror("shmat");
        return EXIT_FAILURE;
    }

    data[0]++;
    shmdt(shmaddr);
    shmid = shmget(key, sizeof(array) + 1, 0666);
    if (shmid == -1) {
        perror("shmget2");
        return EXIT_FAILURE;
    }
    void *shmaddr1 = shmat(shmid, NULL, 0);
    if (shmaddr1 == (void *)-1) {
        perror("shmat");
        return EXIT_FAILURE;
    }
    data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) {
        perror("shmat");
        return EXIT_FAILURE;
    }
    printf("%d", data[0]);
    // shmctl(shmid, IPC_RMID, NULL); ОБЯЗАТЕЛЬНО РАСКОМЕНДИТЬ И ЗАПУСТИТЬ ПО ЗАВЕРШЕНИИ ДЕМОНСТРАЦИИ
    return 0;
}

int fifth_task() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    int key = ftok("/Users/ivan/CLionProjects/Phys_OS/main.c", 0);
    if (key == (key_t)-1) {
        perror("ftok");
        return EXIT_FAILURE;
    }
    int array[] = {0};
    if (pid == 0) {
        int shmid = shmget(key, sizeof(array) + 1, IPC_CREAT|0666);
        if (shmid == -1) {
            perror("shmget1");
            return EXIT_FAILURE;
        }
        void *shmaddr = shmat(shmid, NULL, 0);
        if (shmaddr == (void *)-1) {
            perror("shmat");
            return EXIT_FAILURE;
        }

        char *data = (char *)shmat(shmid, NULL, 0);
        if (data == (char *)-1) {
            perror("shmat");
            return EXIT_FAILURE;
        }
        data[0]++;
        shmdt(shmaddr);
        sleep(1);
    }
    else {
        int shmid = shmget(key, sizeof(array) + 1, IPC_CREAT|0666);
        if (shmid == -1) {
            perror("shmget2");
            return EXIT_FAILURE;
        }
        void *shmaddr1 = shmat(shmid, NULL, 0);
        if (shmaddr1 == (void *)-1) {
            perror("shmat");
            return EXIT_FAILURE;
        }
        char *data = (char *)shmat(shmid, NULL, 0);
        if (data == (char *)-1) {
            perror("shmat");
            return EXIT_FAILURE;
        }
        printf("%d", data[0]);
        sleep(2);
        shmdt(shmaddr1);
        shmctl(shmid, IPC_RMID, NULL);
    }


    return 0;
}

int semafor_1;



int main(int argc, char *argv[])
{
    int sem_id;
    char pathname[1024];
    sprintf(pathname, "%s/%s", abs_path, "main.c");
    key_t key;
    struct sembuf sem;
    if ((key = ftok(pathname, 0)) < 0) {
        perror("ftok");
        return EXIT_FAILURE;
    }
    if ((sem_id = semget(key, 1, 0666 | IPC_CREAT))) {
        perror("semget");
        return EXIT_FAILURE;
    }
    sem.sem_op = -1;
    sem.sem_flg = 0;
    sem.sem_num = 0;
    if (semop(sem_id, &sem, 1) < 0) {
        perror("semop");
        return EXIT_FAILURE;
    }
    printf("%s", "POTREBITEL");
    return 0;
}
