#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main (int argc, char *argv[]) {
    /* check if arguments are present */
    if(argc != 4) {
        fprintf(stderr, "usage: samples file numberfrags maxfragsize\n");
        return EXIT_FAILURE;
    }

    FILE *fd = fopen(argv[1], "r");
    /* check if fopen has errors */
    if(fd == NULL) {
        fprintf(stderr, "%s: cannot open %s: %s\n", argv[0], argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    /* get number of bytes in the file */
    if(fseek(fd, 0, SEEK_END) < 0) {
        fprintf(stderr, "%s: fseek error: %s\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }
    int file_size = ftell(fd) / sizeof(char);
    if(file_size < 0) {
        fprintf(stderr, "%s: ftell error: %s\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }
    if(fseek(fd, 0, SEEK_SET) < 0) {
        fprintf(stderr, "%s: fseek error: %s\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    /* check if max fragment size is larger than the file */
    if(atoi(argv[3]) > file_size) {
        fprintf(stderr, "maxfragsize larger than file\n");
        return EXIT_FAILURE;
    }

    /* alloc a string with maxfragsize as size */
    char *str = (char*)malloc((atoi(argv[3])+1) * sizeof(char));

    /* print fragments */
    srandom(0);
    for(int i = 0; i < atoi(argv[2]); i++) {
        /* never start reading from a place where it doesn't have enough characters to fill maxfragsize */
        int rand = random() % (file_size - atoi(argv[3]));

        /* set fd to be in a random position */
        if(fseek(fd, rand * sizeof(char), SEEK_SET) < 0) { 
            fprintf(stderr, "%s: fseek error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }

        /* store maxfragsize size of the file in the string */
        fread(str, sizeof(char), atoi(argv[3]), fd); 

        /* check for fread error */
        if(ferror(fd)) {
            fprintf(stderr, "%s: fread error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }

        /* replace ASCII characters from 0 to 31 and 127 onwards with a space */
        for(int j = 0; j < atoi(argv[3]); j++) {
            if(str[j] < ' ' || str[j] > '~') {
                str[j] = ' ';
            }
        }

        printf(">%s<\n", str);

        /* return stream to the beginning of the file */
        if(fseek(fd, 0, SEEK_SET) < 0) { 
            fprintf(stderr, "%s: fseek error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }
    fclose(fd);

    return EXIT_SUCCESS;
}

