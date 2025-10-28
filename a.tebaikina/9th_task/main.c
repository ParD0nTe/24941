#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror ("fork failed");
        exit (1);
    }
    // мы внутри дочернего процесса
    if (pid == 0) {
        // execlp заменяет текущий процесс программой "cat"
        // Аргументы:
        //  1. "cat" — имя программы, которую нужно запустить (ищется в $PATH)
        //  2. "cat" — argv[0], имя программы, как она будет называться внутри процесса
        //  3. "testfile.txt" — argv[1], аргумент, который cat воспримет как путь к файлу
        //  4. NULL — обязательный завершающий маркер аргументов
        execlp ("cat", "cat", "testfile.txt", NULL);
        perror("exec failed"); // если exec не сработал
        exit(1);
    } else {
        printf("родитель запущен, ждем завершения дочернего процесса...\n");

        // дать заврешения именно pid,код возврата мне не нужен
        waitpid(pid, NULL, 0);

        printf("дочерний процесс завершился, теперь родитель печатает последнюю строку.\n");
    }
    return 0;
}