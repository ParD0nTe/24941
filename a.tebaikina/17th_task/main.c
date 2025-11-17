#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <termios.h> // для изменения режима терминала

#define MAX_LEN 40

// echo - печатаешь видно
// icanon - собсна включение выключение канонического
int main(void) {
    struct termios old, new;
    tcgetattr(STDIN_FILENO, &old); // читаем текущие настройки
    new = old;

    new.c_lflag &= ~(ICANON | ECHO); // из нового убираем флаги icanon и echo
    new.c_cc[VMIN] = 1; // read() вернётся, как только придёт хотя бы 1 символ
    new.c_cc[VTIME] = 0; // время ожидания бесконечно

    tcsetattr(STDIN_FILENO, TCSANOW, &new); // применить новые настройки сразу

    char line[MAX_LEN] = {0};
    int len = 0;

    while (1) {
        unsigned char ch;
        if (read(STDIN_FILENO, &ch, 1) != 1) break;

        if (ch == 0x04) { // ctrl D (завершает ввод только если строка пуста)
            if (len==0) break;
            continue;
        }

        if (ch == 0x7F) { // erase (backspace)
            if (len > 0) {
                len--;
                write(STDOUT_FILENO, "\b \b", 3); // стереть символ визуально
           }
            continue;
        }

        if (ch == 0x15) { // kill - ctrl U (все удалить)
            while (len > 0) {
                write(STDOUT_FILENO, "\b \b", 3);
                len --;
            }
            continue;
        }

        if (ch == 0x17) { // Ctrl+W - удалить слово
            // стереть пробелы справа
            while (len > 0 && isspace((unsigned char)line[len - 1])) {
                write(STDOUT_FILENO, "\b \b", 3);
                len--;
            }
            // стереть до следующего пробела
            while (len > 0 && !isspace((unsigned char)line[len - 1])) {
                write(STDOUT_FILENO, "\b \b", 3);
                len--;
            }
            continue;
        }

        if (isprint(ch)) { // печатный символ
            if (len < MAX_LEN) { // продолжаем печать
                line[len++] = ch;
                write(STDOUT_FILENO, &ch, 1);
            } else {
                // если слово не помещается — перенос
                if (isspace(ch)) { // перенос тока если там пробел
                    write(STDOUT_FILENO, "\n", 1);
                    len = 0;
                    memset(line, 0, sizeof(line));
                } else {
                    write(STDOUT_FILENO, "\a", 1); // сигнал
                }
            }
        } else {
            write(STDOUT_FILENO, "\a", 1); // непечатаемый символ
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old); // вернуть старые настройки
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}











