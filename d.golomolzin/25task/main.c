#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


// низкая функция чтения для дочернего класса
void read_ (int fd) {

    printf("\nРезультат выполнения ДОЧЕРНЕГО процесса:\n[upper_text]: ");
    // считываем пока можем по одному символу
    char c;
    int bytes;
    while ( (bytes = read(fd, &c, 1)) > 0) {

        putchar( toupper(c) );
    }
    close(fd);
    printf("\n");
}

// низкая функция записи для родительского процесса
void write_ (int fd) {

    char* text = "hello, world and my friend!\n";
    printf("\nЗаписываемый РОДИТЕЛЬСКИМ процессом текст:\n[origin_text]: %s", text);
    write(fd, text, strlen(text));
    close(fd);
}


int main () {

    printf("\n┌────────────────────────────────────┐\n");
    printf("│  Перевод текста в верхний регистр  │\n");
    printf("└────────────────────────────────────┘\n");

    // массив дескприторов
    int mypipe[2];

    // попытка создания канала связи
    if (pipe(mypipe) == -1) {
        perror("pipe");
        return 1;
    }

    
    // создаем дочерний процесс
    pid_t pid;
    pid = fork ();

    // если 0, значит пишем функционал для дочернего
    if (pid == 0) {
        // считываем И сразу переводим в верхний регистр И сразу выводим в канал вывода
        close(mypipe[1]);
        read_(mypipe[0]);
        return 0;
    }
    // иначе функционал для родительского процесса
    else {
        close(mypipe[0]);
        write_(mypipe[1]);

        wait(NULL);
        return 0;
    }
    

    // возвращет массив файловых дескрипторов. 3 для чтения, 4 для записи
    // printf("%d %d\n", mypipe[0], mypipe[1]);
    return 0;
}