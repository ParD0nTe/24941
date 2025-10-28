#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


int count = 0;

// будет писщать при нажатии CTRL^C
void signal_SIGINT() {

    // буквально звук
    printf("\a");

    fflush(NULL);
    count++;
}

// будет завершать работу цикла
void signal_SIGQUIT() {
    
    printf("\nСигнал прозвучал %d раз\n", count);
    exit(0);
}


int main() {

    signal(SIGINT,  &signal_SIGINT);
    signal(SIGQUIT, &signal_SIGQUIT);

    for(;;);
}
