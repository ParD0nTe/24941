#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


int count = 0;

void sigevent(const int sig)
{
    if (sig == SIGINT) // CTRL-С
    {
        write(1, "\a", 1);
        count++;
    }
    else if (sig == SIGQUIT) // "CTRL-\"
    {
        printf("\n%d\n", count);
        exit(0);
    }
}

int main()
{
    struct sigaction sa;

    sa.sa_handler = sigevent;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    signal(SIGINT, sigevent);
    signal(SIGQUIT, sigevent);

    while (1)
    {
        pause();
    }

    return 0;
}
