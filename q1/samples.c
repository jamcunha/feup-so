#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[]) {
    /* check if arguments are present */
    if(argc < 2) {
        printf("usage: samples file numberfrags maxfragsize\n");
        return EXIT_FAILURE;
    }

    srandom(0);
    char *str = (char*)malloc((atoi(argv[3])+1) * sizeof(char));
    FILE *fd = fopen(argv[1], "r");
    fseek(fd, 0, SEEK_END);
    int file_size = ftell(fd) / sizeof(char);
    fseek(fd, 0, SEEK_SET);
    for(int i = 0; i < atoi(argv[2]); i++) {
        int rand = random() % file_size;

        fseek(fd, rand * sizeof(char), SEEK_SET);
        fread(str, sizeof(char), atoi(argv[3]), fd);

        printf(">%s<\n", str);
        fseek(fd, 0, SEEK_SET);
    }
    fclose(fd);

    return EXIT_SUCCESS;
}

