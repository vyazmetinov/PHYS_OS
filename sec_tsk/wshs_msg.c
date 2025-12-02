#include <stdio.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>

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

typedef struct {
    long mtype;
    char mtext[WR_SZ];
} msg_t;

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


void wash(int msg_t_id, int msg_d_id) {
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
            msg_t token;
            if(msgrcv(msg_t_id, (void *)&token, sizeof(token.mtext), 1, 0) == -1){ perror("read_TT"); exit(1); }
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
            msg_t msg;
            msg.mtype = 1;
            snprintf(msg.mtext, sizeof(msg.mtext), "%s:%d\n", type_wash, 1);
            if(msgsnd(msg_d_id, &msg, sizeof(msg.mtext), 0) == -1) { perror("msgsnd"); exit(1); }
        }
    }
    fclose(f);
    msg_t msg;
    msg.mtype = 1;
    snprintf(msg.mtext, sizeof(msg.mtext), "0");
    if(msgsnd(msg_d_id, &msg, sizeof(msg.mtext), 0) == -1) { perror("msgsnd"); exit(1); }
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



void wiper(int msg_t_id, int msg_d_id) {
    msg_t str;

    while (1) {
        if (msgrcv(msg_d_id, &str, sizeof(str.mtext), 1, 0) == -1) { perror("msgrcv"); exit(1); }
        if (strcmp(str.mtext, "0") == 0) {
            break;
        }
        char type[STR_SIZE];
        int amount = 0;
        if (sscanf(str.mtext, " %127[^:]:%d", type, &amount) != 2) {
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
        msg_t token;
        token.mtype = 1;
        token.mtext[0] = 'T';
        if (msgsnd(msg_t_id, &token, sizeof(token.mtext), 0) == -1) { perror("msgsnd"); exit(1); }
    }


}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s wash|wipe\n", argv[0]);
        exit(1);
    }
    key_t key_dish = ftok("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/wshs_msg.c", 0);
    if (key_dish == -1) { perror("ftok"); exit(1); }
    key_t key_table = ftok("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/wshs_msg.c", 1);
    if (key_table == -1) { perror("ftok"); exit(1); }
    int msg_t_id = 0;
    int msg_d_id = 0;
    if (strcmp(argv[1], "wash") == 0) {
        msg_t_id = msgget(key_table, 0666 | IPC_CREAT | IPC_EXCL);
        if (msg_t_id == -1) { perror("msgget"); exit(1); }
        msg_d_id = msgget(key_dish, 0666 | IPC_CREAT | IPC_EXCL);
        if (msg_d_id == -1) { perror("msgget"); exit(1); }
        for (int i = 0; i < TABLE_LIMIT; i++) {
            msg_t msg;
            msg.mtype = 1;
            msg.mtext[0] = 'T';
            if (msgsnd(msg_t_id, &msg, 1, 0) == -1) {
                perror("msgsnd init table");
                exit(1);
            }
        }

        init_washer();
        wash(msg_t_id, msg_d_id);

    }
    else {
        msg_t_id = msgget(key_table, 0666);
        if (msg_t_id == -1) { perror("msgget"); exit(1); }
        msg_d_id = msgget(key_dish, 0666);
        if (msg_d_id == -1) { perror("msgget"); exit(1); }
        init_wiper();
        wiper(msg_t_id, msg_d_id);
        if (msgctl(msg_t_id, IPC_RMID, NULL) == -1) {
            perror("msgctl msg_t_id");
        }
        if (msgctl(msg_d_id, IPC_RMID, NULL) == -1) {
            perror("msgctl msg_d_id");
        }
    }

    return 0;
}
