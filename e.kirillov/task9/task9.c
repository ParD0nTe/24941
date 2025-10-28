#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();
        
    // не получилось создать подпроцесс
    if (pid == -1) {
        perror("Failed create P_PID\n");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс
        execlp("cat", "childProccess", "../task3/task3.c", NULL);
        // ошибка
        perror("execlp failed");
        return 1;
    }
    else {
        // Родительский процесс
        if(wait(NULL) != -1) {
            printf("\nChild process (pid: %d) finished\n", pid);
        }
    }

    return 0;
}