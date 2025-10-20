#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>



// ./main ./prog -V MY=NAME -v

int main (int argc, char* argv[]) {

    if (argc < 2) {
        printf("Программа: %s <program>\n", argv[0]);
        return 1;
    }
    // создаем подпроцесс
    pid_t POTOMOK_PID = fork();

    // если не получилось создать подпроцесс
    if (POTOMOK_PID == -1) {
        perror("fork");
        return 1;
    }
    
    // иначе если == 0, то пишем функционал для дочернего процесса
    if (POTOMOK_PID == 0) {
        // заменяет порожденный процесс собой И при успешном execvp
        // как следствие, другой код в этой части не будет выполнен
        execvp(argv[1], &argv[1]); // более удобная так как принимает просто массив команд
        // если же execvp завершился не успешно, то часть кода выполнится
        perror("Failed execvp");
        return 1;
    }
    // иначе != 0 и это предок
    else {
        int status;
        wait(&status);
        printf("\nДочерний процесс завершился с %d кодом завершения\n\n", status);
    }
    return 0;
}