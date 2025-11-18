#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main () {

    // создаем подпроцесс
    pid_t POTOMOK_PID = fork();

    // если не получилось создать подпроцесс
    if (POTOMOK_PID == -1) {
        perror("Failed create P_PID\n");
        return 1;
    }

    // иначе если == 0, то пишем функционал для дочернего процесса
    if (POTOMOK_PID == 0) {
        // заменяет порожденный процесс собой И при успешном execlp
        // как следствие, другой код в этой части не будет выполнен
        execlp("cat", "cat", "file.txt", NULL);
        // если же execlp завершился не успешно, то часть кода выполнится
        perror("Failed execlp");
        return 1;
    }
    // иначе != 0 и это предок
    else {
        // ждем потомка. NULL - потому что у нас единственный порожденный подпроцесс
        wait(NULL);
        printf("\nЭтот вывод выполнил ГЛАВНЫЙ процесс\nSomething interesting from PARENT\n\n");
    }

    return 0;
}