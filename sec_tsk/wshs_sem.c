#include <stdio.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const int TABLE_LIMIT = 3;

enum {
    PLATE = 0,
    CUP = 1,
    GLASS = 2,
};

enum {
    SEM_WASH = 0,
    SEM_WIPE = 1,
};


int P(int semid, unsigned short num, int val) {
    struct sembuf op = { .sem_num = num, .sem_op = -val, .sem_flg = 0 };
    return semop(semid, &op, 1);
}

int V(int semid, unsigned short num, int val) {
    struct sembuf op = { .sem_num = num, .sem_op = +val, .sem_flg = 0 };
    return semop(semid, &op, 1);
}

int times_wash[3] = {0, 0, 0};

void init_washer() {
    FILE* f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/washer.txt", "r");
    if (!f) { perror("washer.txt"); exit(1); }
    char type[1024];
    int time = 0;
    while (fscanf(f, " %1023[^:]:%d", type, &time) == 2) {
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

void wash(int sem_id) {
    FILE *f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/queue.txt", "r");
    printf("SECCESS\n");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    char type_wash[1024];
    int amount_wash = 0;
    FILE *f_wiper = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/qwipe.txt", "a");
    while (fscanf(f, " %1023[^:]:%d", type_wash, &amount_wash) == 2) {
        for (int i = 0; i < amount_wash; i++) {
            P(sem_id, SEM_WASH, 1);
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

            fprintf(f_wiper, "%s:1\n", type_wash);
            fflush(f_wiper);
            V(sem_id, SEM_WIPE, 1);
        }
    }
    fclose(f);
    fclose(f_wiper);
}

int wipe_time[3] = {0, 0, 0};

void init_wiper() {
    FILE* f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/wiper.txt", "r");
    if (!f) { perror("wiper.txt"); exit(1); }
    char type[1024];
    int time = 0;
    while (fscanf(f, " %1023[^:]:%d", type, &time) == 2) {
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



void wiper(int sem_id) {
    FILE* f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/qwipe.txt", "r");
    if (!f){ perror("fopen"); exit(1); }

    char type[1024];
    int amount = 0;
    while (1) {
        printf("ЖДУ ТОРМОЗА\n");
        if (P(sem_id, SEM_WIPE, 1) == -1) {
            perror("P SEM_WIPE");
            break;
        }

        if (fscanf(f, " %1023[^:]:%d", type, &amount) != 2) {
            perror("fscanf qwipe");
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

        V(sem_id, SEM_WASH, 1);
    }

    fclose(f);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s wash|wipe\n", argv[0]);
        exit(1);
    }
    key_t sem_key = ftok("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/wshs_sem.c", 0);
    if (sem_key == -1) { perror("ftok"); exit(1); }
    int sem_id = semget(sem_key, 2, 0666 | IPC_CREAT | IPC_EXCL);
    if (sem_id == -1) {
        if (errno == EEXIST) {
            sem_id = semget(sem_key, 2, 0);
        }
        if (sem_id == -1){ perror("semget"); exit(1); }
    }
    else {
        semctl(sem_id, SEM_WASH, SETVAL, TABLE_LIMIT);
        semctl(sem_id, SEM_WIPE, SETVAL, 0);
    }
    if (strcmp(argv[1], "wash") == 0) { init_washer(); wash(sem_id); }
    else { init_wiper(); wiper(sem_id); semctl(sem_id, 0, IPC_RMID); }

    return 0;

}