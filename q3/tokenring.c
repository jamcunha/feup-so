#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

int main (int argc, char *argv[]) {
    /* check number of args */
    if(argc != 4) {
        printf("usage: tokenring number_process probability number_seconds\n");
        return EXIT_FAILURE;
    }

    char *loc = (char*)malloc(50 * sizeof(char));
    for(int i = 1; i <= atoi(argv[1]); i++) {
        if(i == atoi(argv[1]))
            sprintf(loc, "pipe%dto1", i);
        else
            sprintf(loc, "pipe%dto%d", i, i+1);

        mkfifo(loc, 666);
        
        loc[0] = '\0';
    }
    free(loc);

    pid_t pids[atoi(argv[1])];
    char *write = (char*)malloc(50 * sizeof(char));
    char *read = (char*)malloc(50 * sizeof(char));
    for(int i = 1; i <= atoi(argv[1]); i++) {
        if((pids[i-1] = fork()) < 0) {
            fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        } else if(pids[i-1] == 0) {
            if(i == atoi(argv[1]))
                sprintf(loc, "pipe%dto1", i);
            else
                sprintf(loc, "pipe%dto%d", i, i+1);
            printf("child %d: loc - %s\n", i, loc);

            for(;;) {
                continue;
            }

            exit(EXIT_SUCCESS);
        }
    }

    for(int i = 0; i < atoi(argv[1]); i++) {
        if(waitpid(pids[i], NULL, 0) < 0) {
            fprintf(stderr, "%s: waitpid error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

