#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t beep_count = 0;
static volatile sig_atomic_t quit_flag = 0;

static void on_sigint(int signo) {
    (void) signo; // переменная signo не используется
    const char bel = '\a'; // это объявление символа BEL
    write(STDOUT_FILENO, &bel, 1); // сам вывод звука
    ++beep_count;
}

static void on_sigquit(int signo) {
    (void) signo;
    quit_flag = 1;
}

// когда придет сигнал, вызови handler
static void set_handler(int sig, void (*handler)(int)) {
    struct sigaction sa = {0};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask); // во время исполнения обработчика не блокируй никакие другие сериалы
    sa.sa_flags = SA_RESTART; // если сигнал прервал системный вызов (например, read() или pause()), автоматически перезапусти его
    if (sigaction(sig, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    // установим обработчики сигналов
    set_handler(SIGINT,  on_sigint);
    set_handler(SIGQUIT, on_sigquit);
    fprintf(stderr,
            "Запущено. Нажимай ctrl+C — будет звуковой сигнал.\n"
            "Нажми SIGQUIT (обычно Ctrl+\\), чтобы вывести счётчик и завершить.\n");

    // Ждём сигналы
    while (!quit_flag) {
        pause(); // спим, пока не придёт сигнал
    }

    printf("\nВсего звуковых сигналов: %d\n", (int)beep_count);
    return 0;
}