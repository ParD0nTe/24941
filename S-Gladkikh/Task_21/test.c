#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t beep_count = 0;

void handler(int sig) {
    if (sig == SIGINT) {
        beep_count++;
        write(STDOUT_FILENO, "BEEP!\n", 6);
    } else if (sig == SIGQUIT) {
        printf("\nBeeped %d times. Exiting...\n", beep_count);
        exit(0);
    }
}

int main() {
    signal(SIGINT, handler);
    signal(SIGQUIT, handler);
    
    printf("Test program. Press Ctrl+C for beep, Ctrl+\\ to exit.\n");
    
    while (1) {
        pause();
    }
    
    return 0;
}