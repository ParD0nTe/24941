#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 

volatile int count = 0;

void handle_sigint(int sig)
{
    count++;
    printf("\a");
    fflush(stdout);
}

// Обработчик сигнала SIGQUIT (CTRL-\)
void handle_sigquit(int sig)
{
    printf("\nВы вызвали звуковой сигнал %d раз(а).\n", count);
    exit(0);
}

int main()
{
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, handle_sigquit);

    printf("Программа запущена. Нажмите CTRL-C для звукового сигнала.\n");
    printf("Нажмите CTRL-\\ для завершения и вывода статистики.\n");

    while (1)
    {
        // Просто ждём сигналов
    }
    return 0;
}