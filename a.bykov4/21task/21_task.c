#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int sig_count = 0;

void sigint() {
    printf("\a");
    fflush(stdout);
    sig_count++;
}

void sigquit() {
    printf("\nКоличество писков: %d\n", sig_count);
    exit(0);
}


int main() {

    signal(SIGINT,  &sigint);
    signal(SIGQUIT, &sigquit);

    while(1){
        pause();
    }

}
