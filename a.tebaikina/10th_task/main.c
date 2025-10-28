#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror ("lack of arguments");
        exit (1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        // дочерний процесс: запускаем указанную команду
        // pапусти программу ls и передай ей аргументы ["ls", "-l", "/etc"]».
        execvp(argv[1], &argv[1]);
        perror("ошибка execvp");
        exit(1);
    } else {
        // родительский процесс: ждём завершения ребёнка
        int status;
        waitpid(pid, &status, 0);
        // проверяем, как завершился дочерний процесс
        if (WIFEXITED(status)) {
            // завершился нормально (exit/return)
            printf("команда завершилась с кодом %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            // убит сигналом (kill, Ctrl+C и т.п.)
            printf("команда завершилась из-за сигнала %d\n", WTERMSIG(status));
        } else {
            // другой случай (редко)
            printf("команда завершилась необычным образом.\n");
        }
    }
    return 0;
}