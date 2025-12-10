#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

enum {
    TABLE_LIMIT = 3,
    STR_SIZE    = 128
};

enum {
    PLATE = 0,
    CUP   = 1,
    GLASS = 2
};

#define SOCK_PATH "/tmp/wshs_sk.sock"

static int sock_fd  = -1;
static int conn_fd  = -1;

int times_wash[3] = {0, 0, 0};
int wipe_time[3]  = {0, 0, 0};



void init_washer(void)
{
    struct sockaddr_un addr;
    FILE *f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/washer.txt", "r");
    if (!f) {
        perror("washer.txt");
        exit(1);
    }

    char type[STR_SIZE];
    int time = 0;
    while (fscanf(f, " %127[^:]:%d", type, &time) == 2) {
        if (strcmp(type, "CUP") == 0) {
            times_wash[CUP] = time;
        } else if (strcmp(type, "GLASS") == 0) {
            times_wash[GLASS] = time;
        } else {
            times_wash[PLATE] = time;
        }
    }
    fclose(f);

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(1);
    }
}

void init_wiper(void)
{
    struct sockaddr_un addr;

    FILE *f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/wiper.txt", "r");
    if (!f) {
        perror("wiper.txt");
        exit(1);
    }

    char type[STR_SIZE];
    int time = 0;
    while (fscanf(f, " %127[^:]:%d", type, &time) == 2) {
        if (strcmp(type, "CUP") == 0) {
            wipe_time[CUP] = time;
        } else if (strcmp(type, "GLASS") == 0) {
            wipe_time[GLASS] = time;
        } else {
            wipe_time[PLATE] = time;
        }
    }
    fclose(f);

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCK_PATH);

    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sock_fd);
        exit(1);
    }

    if (listen(sock_fd, 1) == -1) {
        perror("listen");
        close(sock_fd);
        exit(1);
    }

    conn_fd = accept(sock_fd, NULL, NULL);
    if (conn_fd == -1) {
        perror("accept");
        close(sock_fd);
        exit(1);
    }
}


void wash(void)
{
    FILE *f = fopen("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/queue.txt", "r");
    if (!f) {
        perror("queue.txt");
        exit(1);
    }
    FILE *sock = fdopen(sock_fd, "r+");
    if (!sock) {
        perror("fdopen washer");
        fclose(f);
        close(sock_fd);
        exit(1);
    }

    char type_wash[STR_SIZE];
    int amount_wash = 0;

    while (fscanf(f, " %127[^:]:%d", type_wash, &amount_wash) == 2) {
        for (int i = 0; i < amount_wash; i++) {
            int c = fgetc(sock);
            if (c == EOF) {
                fprintf(stderr, "washer: unexpected EOF while waiting for token\n");
                fclose(f);
                fclose(sock);
                exit(1);
            }
            if (c != 'T') {
                fprintf(stderr, "washer: expected 'T', got '%c'\n", c);
                fclose(f);
                fclose(sock);
                exit(1);
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

            /* отправляем одну «готовую» посуду */
            fprintf(sock, "%s\n", type_wash);
            fflush(sock);
        }
    }

    fprintf(sock, "END\n");
    fflush(sock);

    fclose(f);
    fclose(sock);
}

void wipe(void)
{
    FILE *sock = fdopen(conn_fd, "r+");
    if (!sock) {
        perror("fdopen wiper");
        close(conn_fd);
        close(sock_fd);
        exit(1);
    }

    for (int i = 0; i < TABLE_LIMIT; i++) {
        if (fputc('T', sock) == EOF) {
            perror("wiper fputc init");
            fclose(sock);
            close(sock_fd);
            exit(1);
        }
    }
    fflush(sock);

    char line[STR_SIZE];

    while (1) {
        if (!fgets(line, sizeof(line), sock)) {
            if (ferror(sock)) perror("wiper fgets");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "END") == 0) {
            break;
        }

        if (strcmp(line, "CUP") == 0) {
            printf("ВЫТИРАЮ ЧАШКУ\n");
            sleep(wipe_time[CUP]);
        } else if (strcmp(line, "GLASS") == 0) {
            printf("ВЫТИРАЮ СТАКАН\n");
            sleep(wipe_time[GLASS]);
        } else {
            printf("ВЫТИРАЮ ТАРЕЛКУ\n");
            sleep(wipe_time[PLATE]);
        }

        if (fputc('T', sock) == EOF) {
            perror("wiper fputc token");
            break;
        }
        fflush(sock);
    }

    fclose(sock);
    close(sock_fd);
    unlink(SOCK_PATH);
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s wash|wipe\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "wash") == 0) {
        init_washer();
        wash();
    } else if (strcmp(argv[1], "wipe") == 0) {
        init_wiper();
        wipe();
    } else {
        fprintf(stderr, "Unknown mode: %s (use wash or wipe)\n", argv[1]);
        return 1;
    }

    return 0;
}