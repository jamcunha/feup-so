#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main (int argc, char *argv[]) {
    /* check if arguments are present */
    if(argc < 2) {
        printf("usage: txt2epub file [file]\n");
        return EXIT_FAILURE;
    }

    pid_t pids[argc-1];
    for(int i = 1; i < argc; i++) {
        if((pids[i-1] = fork()) < 0) {
            fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        } else if(pids[i-1] == 0) {
            printf("[pid%d] converting %s ...\n", getpid(), argv[i]);

            char *epub_ext = ".epub";
            char *epub = (char*)malloc(strlen(argv[i])+1);
            strcpy(epub, argv[i]);
            epub[strlen(epub)-4] = '\0';
            strcat(epub, epub_ext);

            /* adding pandoc args to an array to use execvp */
            char **exec = (char**)malloc(5 * sizeof(char*));
            exec[0] = "pandoc";
            exec[1] = argv[i];
            exec[2] = "-o";
            exec[3] = epub;
            exec[4] = NULL;

            execvp(exec[0], exec);

            /* execvp error */
            fprintf(stderr, "%s: couldn't convert file to epub: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    /* wait for all child processes to end */
    for(int i = 0; i < argc-1; i++) {
        if(waitpid(pids[i], NULL, 0) < 0) {
            fprintf(stderr, "%s: waitpid error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }

    /* zip epubs */
    pid_t pid;
    if((pid = fork()) < 0) {
        fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    } else if(pid == 0) {
        system("zip ebooks.zip *.epub");
        if(system("zip ebooks.zip *.epub") < 0) {
            fprintf(stderr, "%s: couldn't compress epub files: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    } else {
        if(waitpid(pid, NULL, 0) < 0) {
            fprintf(stderr, "%s: waitpid error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}

