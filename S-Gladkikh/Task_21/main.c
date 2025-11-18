#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t beep_count = 0;

void sigint_handler() {
    beep_count++;
    write(1,"\a",1);
}

void sigquit_handler() {
    char buffer[50];
    int len = printf("%d\n",beep_count);
    exit(0);
}

int main() {
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    
    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    while (1) {
        pause();
    }
    return 0;
}