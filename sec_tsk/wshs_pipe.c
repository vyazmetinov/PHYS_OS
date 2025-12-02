#include <stdio.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

enum {
    TABLE_LIMIT = 3,
    STR_SIZE = 128,
    WR_SZ = STR_SIZE + sizeof(int),
};


enum {
    PLATE = 0,
    CUP = 1,
    GLASS = 2,
};

int times_wash[3] = {0, 0, 0};

void init_washer() {
    FILE* f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/washer.txt", "r");
    if (!f) { perror("washer.txt"); exit(1); }
    char type[STR_SIZE];
    int time = 0;
    while (fscanf(f, " %127[^:]:%d", type, &time) == 2) {
        if (strcmp(type, "CUP") == 0) {
            times_wash[CUP] = time;
        }
        else if (strcmp(type, "GLASS") == 0) {
            times_wash[GLASS] = time;
        }
        else {
            times_wash[PLATE] = time;
        }
    }
    fclose(f);
}

char buf_dish[WR_SZ * TABLE_LIMIT + 1];

void wash(int *p_dish, int *p_table) {
    FILE *f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/queue.txt", "r");
    printf("SUCCESS\n");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    char type_wash[STR_SIZE];
    int amount_wash = 0;
    while (fscanf(f, " %127[^:]:%d", type_wash, &amount_wash) == 2) {
        for (int i = 0; i < amount_wash; i++) {
            char token;
            if(read(p_table[0], &token, 1) != 1){ perror("read_TT"); exit(1); }
            if (strcmp(type_wash, "CUP") == 0) {
                printf("МОЮ ЧАШКУ\n");
                sleep(times_wash[CUP]);
            } else if (strcmp(type_wash, "GLASS") == 0) {
                printf("МОЮ СТАКАН\n");
                sleep(times_wash[GLASS]);
            } else {
                printf("МОЮ ТАРЕЛКУ\n");
                sleep(times_wash[PLATE]);
            }
            int len = snprintf(buf_dish, WR_SZ, "%s:%d\n", type_wash, 1);
            if (len < 0 || len >= (int)sizeof(buf_dish)) { perror("snprintf"); exit(1); }
            if(write(p_dish[1], buf_dish, len) != len) { perror("write_PD"); exit(1); }
        }
    }
    fclose(f);
    close(p_dish[1]);
}

int wipe_time[3] = {0, 0, 0};

void init_wiper() {
    FILE* f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/wiper.txt", "r");
    if (!f) { perror("wiper.txt"); exit(1); }
    char type[STR_SIZE];
    int time = 0;
    while (fscanf(f, " %127[^:]:%d", type, &time) == 2) {
        if (strcmp(type, "CUP") == 0) {
            wipe_time[CUP] = time;
        }
        else if (strcmp(type, "GLASS") == 0) {
            wipe_time[GLASS] = time;
        }
        else {
            wipe_time[PLATE] = time;
        }
    }
    fclose(f);
}



void wiper(int *p_dish, int *p_table) {
    char str[WR_SZ];
    while (1) {
        ssize_t n = read(p_dish[0], str, sizeof(str) - 1);
        if (n == 0) break;
        if (n < 0) { perror("read p_dish"); break; }
        str[n] = '\0';

        char type[STR_SIZE];
        int amount = 0;
        if (sscanf(str, " %127[^:]:%d", type, &amount) != 2) {
            perror("scanf qwipe");
            break;
        }

        if (strcmp(type, "CUP") == 0) {
            printf("ВЫТИРАЮ ЧАШКУ\n");
            sleep(wipe_time[CUP]);
        }
        else if (strcmp(type, "GLASS") == 0) {
            printf("ВЫТИРАЮ СТАКАН\n");
            sleep(wipe_time[GLASS]);
        }
        else {
            printf("ВЫТИРАЮ ТАРЕЛКУ\n");
            sleep(wipe_time[PLATE]);
        }
        if(write(p_table[1], "1", 1) != 1) { perror("write_PT"); break; }
    }

}


int main() {
    int p_dish[2];
    int p_table[2];
    if (pipe(p_dish) == -1) { perror("pipe"); exit(1); }
    if (pipe(p_table) == -1) { perror("pipe"); exit(1); }
    for (int i = 0; i < TABLE_LIMIT; i++) {
        if (write(p_table[1], "1", 1) != 1) { perror("init_table_pipe"); exit(1); }
    }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0) {
        close(p_dish[0]);
        close(p_table[1]);
        init_washer();
        wash(p_dish, p_table);
        exit(0);
    }
    else {
        close(p_dish[1]);
        close(p_table[0]);
        init_wiper();
        wiper(p_dish, p_table);
        wait(NULL);
    }
    return 0;
}
