#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2]; // массив из двух дескрипторов - fd[0] для чтения и fd[1] для записи
    pid_t pid; // переменная для хранения PID потомка

    // создаем программный канал
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // создаем потомка
    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // блок для потомка
    if (pid == 0) {
        close(fd[1]); // закрываем конец для записи

        char buffer[256]; // буфер для данных, получаемых из канала
        ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);
        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        buffer[n] = '\0'; // превращаем в строку

        // переводим в верхний регистр
        for (int i = 0; buffer[i] != '\0'; i++) {
            buffer[i] = toupper((unsigned char) buffer[i]);
        }

        printf("потомок получил и преобразовал:\n%s\n", buffer);
        close(fd[0]); // закрываем конец для чтения
        exit(EXIT_SUCCESS);// завершаем процесс потомка

    } else {
        // блок для родителя
        close(fd[0]);// закрываем конец для чтения

        const char *text = "Hello, World! Mixed Case Example.\n"; // исходный текст
        write(fd[1], text, strlen(text)); // записываем текст в канал

        close(fd[1]); // закрываем запись

        wait(NULL); // ждем завепшения потомка
    }

    return 0;
}
