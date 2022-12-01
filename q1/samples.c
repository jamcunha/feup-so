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
    fseek(fd, 0, SEEK_END);
    int file_size = ftell(fd) / sizeof(char);
    fseek(fd, 0, SEEK_SET);

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
        fseek(fd, rand * sizeof(char), SEEK_SET); // set fd to be in a random position
        fread(str, sizeof(char), atoi(argv[3]), fd); // store maxfragsize size of the file in the string

        /* replace ASCII characters from 0 to 31 and 127 onwards with a space */
        for(int j = 0; j < atoi(argv[3]); j++) {
            if(str[j] < ' ' || str[j] > '~') {
                str[j] = ' ';
            }
        }

        printf(">%s<\n", str);
        fseek(fd, 0, SEEK_SET); // return stream to the beginning of the file
    }
    fclose(fd);

    return EXIT_SUCCESS;
}

