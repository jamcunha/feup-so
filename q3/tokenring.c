#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
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

        if((mkfifo(loc, 0666)) < 0) {
            fprintf(stderr, "%s: mkfifo error: %s", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    free(loc);

    int prob = 1 / atof(argv[2]);
    srandom(0);

    printf("%d\n", prob);

    int val = 0;

    pid_t pids[atoi(argv[1])];
    char *write_pipe = (char*)malloc(50 * sizeof(char));
    char *read_pipe = (char*)malloc(50 * sizeof(char));
    for(int i = 1; i <= atoi(argv[1]); i++) {
        if((pids[i-1] = fork()) < 0) {
            fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        } else if(pids[i-1] == 0) {
            if(i == atoi(argv[1])) {
                sprintf(write_pipe, "pipe%dto1", i);
                sprintf(read_pipe, "pipe%dto%d", i-1, i);
            } else if(i == 1) {
                sprintf(write_pipe, "pipe%dto%d", i, i+1);
                sprintf(read_pipe, "pipe%dto1", atoi(argv[1]));
            } else {
                sprintf(write_pipe, "pipe%dto%d", i, i+1);
                sprintf(read_pipe, "pipe%dto%d", i-1, i);
            }

            /* store pipes in an array */
            int fd[2];

            if(i == 1) {
                if((fd[1] = open(write_pipe, O_WRONLY)) < 0) {
                    fprintf(stderr, "%s: pipe opening error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                val++;

                if(write(fd[1], &val, sizeof(int)) < 0) {
                    fprintf(stderr, "%s: write error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(fd[1]);
            }

            for(;;) {

                /* read value from previous process */
                if((fd[0] = open(read_pipe, O_RDONLY)) < 0) {
                    fprintf(stderr, "%s: pipe opening error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                if(read(fd[0], &val, sizeof(int)) < 0) {
                    fprintf(stderr, "%s: read error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(fd[0]);

                val++; // increments value
                
                int rand = random() % prob;
                if(rand == 1) {
                    printf("[p%d] lock on token (val = %d)\n", i, val);
                    sleep(atoi(argv[3]));
                    printf("[p%d] unlock token\n", i);
                }

                /* writes value to next process */
                if((fd[1] = open(write_pipe, O_WRONLY)) < 0) {
                    fprintf(stderr, "%s: pipe opening error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }
                
                if(write(fd[1], &val, sizeof(int)) < 0) {
                    fprintf(stderr, "%s: write error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(fd[1]);
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

