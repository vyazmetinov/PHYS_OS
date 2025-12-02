#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <string.h>
#include <sys/semaphore.h>


void getter(int n) {
    int running = 0;
    int limit = n;
    char cmd[1024];
    while (fgets(cmd, sizeof(cmd), stdin)) {
        printf("Program get %s", cmd);
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            running--;
        }

        if (running >= limit) {
            printf("Limit of commands reached\n");
            pid = wait(&status);
            if (pid > 0) running--;
        }

        pid = fork();
        if (pid == 0) {
            system(cmd);
            _exit(127); // если exec не удался
        } else if (pid > 0) {
            running++;
        } else {
            perror("fork");
        }
    }
}

int main(int argc, char **argv) {
    int n = atoi(argv[1]);
    key_t sem_k = ftok("/Users/ivan/CLionProjects/Phys_OS/sec_tsk/runsim.c", 0);
    getter(n);
}

