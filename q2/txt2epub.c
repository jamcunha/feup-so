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
        fprintf(stderr, "usage: txt2epub file [file]\n");
        return EXIT_FAILURE;
    }

    /* check if all files exists */
    FILE *fd;
    for(int i = 1; i < argc; i++) {
        fd = fopen(argv[i], "r");
        if(fd == NULL) {
            fprintf(stderr, "%s: file %s doesn't exist\n", argv[0], argv[i]);
            exit(EXIT_FAILURE);
        }
        fclose(fd);
    }

    /* store epubs names */
    char *epubs[argc-1];
    char *epub_ext = ".epub";
    for(int i = 1; i < argc; i++) {
        epubs[i-1] = (char*)malloc((strlen(argv[i])+2) * sizeof(char));
        strcpy(epubs[i-1], argv[i]);
        epubs[i-1][strlen(epubs[i-1])-4] = '\0';
        strcat(epubs[i-1], epub_ext);
    }

    pid_t pids[argc-1];
    for(int i = 1; i < argc; i++) {
        if((pids[i-1] = fork()) < 0) { // check for fork errors
            fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        } else if(pids[i-1] == 0) { // child
            printf("[pid%d] converting %s ...\n", getpid(), argv[i]);

            /* executing pandoc */
            execlp("pandoc", "pandoc", argv[i], "-o", epubs[i-1], "--quiet", NULL);

            /* execlp error */
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
    if((pid = fork()) < 0) { // check for fork errors
        fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    } else if(pid == 0) { // child
        char *zip[argc+3];
        zip[0] = "zip";
        zip[1] = "ebooks.zip";

        /* store epubs names in zip array */
        for(int i = 0; i < argc-1; i++) {
            zip[i+2] = malloc((strlen(epubs[i])+1) * sizeof(char));
            strcpy(zip[i+2], epubs[i]);
        }

        zip[argc+2] = "--quiet";
        zip[argc+3] = NULL;

        /* executing zip */
        execvp(zip[0], zip);

        /* execvp error */
        fprintf(stderr, "%s: couldn't compress epub files: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* wait for child to end */
        if(waitpid(pid, NULL, 0) < 0) {
            fprintf(stderr, "%s: waitpid error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}

