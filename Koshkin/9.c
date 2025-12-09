#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Дочерний процесс
        execlp("cat", "cat", "secret.txt", NULL);
        perror("execlp failed");
        exit(1);
    } else if (pid > 0) {
        // Родительский процесс
        printf("Родитель: дочерний процесс запущен с PID=%d\n", pid);
        printf("Родитель: выполняю свою работу...\n");
        
        // Ждем завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);  // 0 = блокирующее ожидание
        
        // Эта строка выведется ПОСЛЕ завершения cat
        printf("Родитель: дочерний процесс завершился\n");
        
        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork failed");
        return 1;
    }
    
    return 0;
}