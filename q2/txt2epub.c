#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (int argc, char *argv[]) {
    /* check if arguments are present */
    if(argc < 2) {
        printf("usage: txt2epub file [file]\n");
        return EXIT_FAILURE;
    }

    pid_t pids[argc-1];
    for(int i = 1; i < argc; i++) {
        if((pids[i-1] = fork()) == -1) {
            perror("fork");
            return EXIT_FAILURE;
        } else if(pids[i-1] == 0) {
            printf("[pid%d] converting %s ...\n", getpid(), argv[i]);

            char *epub_ext = ".epub";
            char *epub = (char*)malloc(strlen(argv[i])+1);
            strcpy(epub, argv[i]);
            epub[strlen(epub)-4] = '\0';
            strcat(epub, epub_ext);

            /* adding pandoc args to an array to use execvp */
            char **exec = (char**)malloc(4 * sizeof(char*));
            exec[0] = "pandoc";
            exec[1] = argv[i];
            exec[2] = "-o";
            exec[3] = epub;
            exec[4] = NULL;

            execvp(exec[0], exec);
        }
    }

    /* wait for all child processes to end */
    for(int i = 0; i < argc-1; i++) {
        if(waitpid(pids[i], NULL, 0) == -1) {
            perror("wait");
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}

