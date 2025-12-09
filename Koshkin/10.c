#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Проверяем, что передано хотя бы два аргумента
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        exit(1);
    }

    // Создаем дочерний процесс
    pid_t pid = fork();
    
    if (pid == -1) {
        // Ошибка при создании процесса
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс
        // Выполняем команду с аргументами
        execvp(argv[1], &argv[1]);
        
        // Если execvp вернул управление, значит произошла ошибка
        perror("execvp");
        exit(1);
    } else {
        // Родительский процесс
        int status;
        
        // Ждем завершения дочернего процесса
        pid_t waited_pid = waitpid(pid, &status, 0);
        
        if (waited_pid == -1) {
            // Ошибка при ожидании
            perror("waitpid");
            exit(1);
        }
        
        // Проверяем, как завершился процесс и выводим код завершения
        if (WIFEXITED(status)) {
            printf("Процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Процесс завершился по сигналу: %d\n", WTERMSIG(status));
        } else {
            printf("Процесс завершился ненормально\n");
        }
        
        return 0;
    }
}