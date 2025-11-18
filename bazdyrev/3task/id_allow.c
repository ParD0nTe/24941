#include <stdio.h>
#include <errno.h>
#include <unistd.h>

void printref(void) {
    printf("Real: %d\n", getuid());
    printf("Effective: %d\n", geteuid());
}

int main(void) {
    printref();

    FILE* str = fopen("input.txt", "r");
    if (str == NULL) {
        perror("Blank file\n");
    } else {
        printf("Success\n");
        fclose(str);
    }

    if (setuid(getuid()) == -1) {
        perror("setuid failed");
    }

    printref();

    FILE* fn = fopen("input.txt", "r");
    if (fn == NULL) {
        perror("Blank file\n");
    } 
    else {
        printf("Success\n");
    }
}
