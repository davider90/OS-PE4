#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h> 
#include <sys/stat.h>

// The WISh main
int main(int argc, char **argv) {
    
    int file = open("file.txt", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

    if (file < 0) {
        printf("%d\n", file);
        printf("%s\n", strerror(errno));
        exit(1);
    } 

    dup2(file, 1);

    close(file);

    printf("En helt helt helt annen tekst.\n");


    return 0;
}