#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sched.h>


enum {
    TABLE_LIMIT = 3,
    STR_SIZE = 128,
    WR_SZ = STR_SIZE + sizeof(int),
};

typedef struct {
    char buf[TABLE_LIMIT][WR_SZ];     // Массив для хранения данных
    int head;       // Индекс для чтения (голова очереди)
    int tail;       // Индекс для записи (хвост очереди)
} shm_ring_t;

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


void wash(shm_ring_t *table) {
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
            while (strcmp(table->buf[table->head], "0") != 0) {
                sched_yield();
            }
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
            snprintf(table->buf[table->head], WR_SZ, "%s:%d\n", type_wash, 1);
            table->head = (table->head + 1) % TABLE_LIMIT;
        }
    }
    fclose(f);
    snprintf(table->buf[table->head], sizeof(table->buf[table->head]), "-1");
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



void wiper(shm_ring_t *table) {

    while (1) {
        while (strcmp(table->buf[table->tail], "0") == 0) {
            sched_yield();
        }
        if (strcmp(table->buf[table->tail], "-1") == 0) {
            break;
        }
        char type[STR_SIZE];
        int amount = 0;
        if (sscanf(table->buf[table->tail], " %127[^:]:%d", type, &amount) != 2) {
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
        strcpy(table->buf[table->tail], "0");
        table->tail = (table->tail + 1) % TABLE_LIMIT;

    }


}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s wash|wipe\n", argv[0]);
        exit(1);
    }
    key_t key = ftok("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/wshs_shm.c", 0);
    if (key == -1) { perror("ftok"); exit(1); }
    int shmid = 0;
    if (strcmp(argv[1], "wash") == 0) {
        shmid = shmget(key, sizeof(shm_ring_t), 0666 | IPC_CREAT);
        if (shmid == -1) { perror("shmget"); exit(1); }
        shm_ring_t *table = (shm_ring_t *) shmat(shmid, 0, 0);
        if (table == (void *) -1) { perror("shmat"); exit(1); }
        for (int i = 0; i < TABLE_LIMIT; i++) {
            strcpy(table->buf[i], "0");
        }
        table->head = 0;
        table->tail = 0;
        init_washer();
        wash(table);
        shmctl(shmid, IPC_RMID, NULL);
    }
    else {
        shmid = shmget(key, sizeof(shm_ring_t), 0666);
        if (shmid == -1) { perror("shmget"); exit(1); }
        shm_ring_t *table = (shm_ring_t *) shmat(shmid, 0, 0);
        if (table == (void *) -1) { perror("shmat"); exit(1); }
        init_wiper();
        wiper(table);
        shmdt(table);
    }

    return 0;
}
