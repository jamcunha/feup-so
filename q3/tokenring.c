#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

int main (int argc, char *argv[]) {
    /* check number of args */
    if(argc != 4) {
        fprintf(stderr, "usage: tokenring number_process probability number_seconds\n");
        return EXIT_FAILURE;
    }

    /* check if probablitity is between zero and one */
    if(atof(argv[2]) > 1 && atof(argv[2]) < 0) {
        fprintf(stderr, "probability must be between 0 and 1\n");
        return EXIT_FAILURE;
    }

    /* check if number of processes is more than one */
    if(atoi(argv[1]) < 2) {
        fprintf(stderr, "must have more than one process\n");
        return EXIT_FAILURE;
    }

    /* check if number of seconds is a positive number */
    if(atoi(argv[3]) < 0) {
        fprintf(stderr, "number of seconds must be positive\n");
        return EXIT_FAILURE;
    }

    /* create named pipes */
    char *loc = (char*)malloc(50 * sizeof(char));
    for(int i = 1; i <= atoi(argv[1]); i++) {
        if(i == atoi(argv[1]))
            sprintf(loc, "pipe%dto1", i);
        else
            sprintf(loc, "pipe%dto%d", i, i+1);

        if((mkfifo(loc, 0666)) < 0) {
            fprintf(stderr, "%s: mkfifo error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    free(loc);

    int token = 0;

    /* create processes */
    pid_t pids[atoi(argv[1])];
    char *write_pipe = (char*)malloc(50 * sizeof(char));
    char *read_pipe = (char*)malloc(50 * sizeof(char));
    for(int i = 1; i <= atoi(argv[1]); i++) {
        if((pids[i-1] = fork()) < 0) {
            fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        } else if(pids[i-1] == 0) {
            /* choose named pipe to use as read and write */
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

            srandom(getpid());

            /* store pipes in an array */
            int fd[2];

            /* use only the write pipe in the first process in order to avoid deadlock */
            if(i == 1) {
                /* open write pipe */
                if((fd[1] = open(write_pipe, O_WRONLY)) < 0) {
                    fprintf(stderr, "%s: pipe opening error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                /* end the program if token is the maximum integer value */
                if(token == INT_MAX)
                    exit(EXIT_SUCCESS);
                else
                    token++; // increments value

                /* write value to pipe */
                if(write(fd[1], &token, sizeof(int)) < 0) {
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

                if(read(fd[0], &token, sizeof(int)) < 0) {
                    fprintf(stderr, "%s: read error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(fd[0]);

                token++; // increments value
                
                /* randlomly lock according to a probability */
                int rand = random() % 100;
                if(rand < 100 * atof(argv[2])) {
                    printf("[p%d] lock on token (val = %d)\n", i, token);
                    sleep(atoi(argv[3]));
                    printf("[p%d] unlock token\n", i);
                }

                /* writes value to next process */
                if((fd[1] = open(write_pipe, O_WRONLY)) < 0) {
                    fprintf(stderr, "%s: pipe opening error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }
                
                if(write(fd[1], &token, sizeof(int)) < 0) {
                    fprintf(stderr, "%s: write error: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(fd[1]);
            }

            exit(EXIT_SUCCESS);
        }
    }

    /* wait for all childs to end */
    for(int i = 0; i < atoi(argv[1]); i++) {
        if(waitpid(pids[i], NULL, 0) < 0) {
            fprintf(stderr, "%s: waitpid error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }

    printf("Reached max limit of an integer. Exiting the program\n");

    return EXIT_SUCCESS;
}

