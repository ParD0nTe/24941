#include <unistd.h>
#include <sys/termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define M_CERASE   0x7F  // Backspace (обычно Ctrl+H или Backspace)
#define M_CKILL    0x15  // Ctrl+U (удалить всю строку)
#define M_CWERASE  0x17  // Ctrl+W (удалить слово)
#define M_CEOF     0x04  // Ctrl+D (завершить программу)

#define LINE_LENGTH 40
struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
 
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;  // Ждать минимум 1 символ
    raw.c_cc[VTIME] = 0; // без таймаута
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    enableRawMode();

    char c;
    static char line[LINE_LENGTH + 1];

    while (read(STDIN_FILENO, &c, 1) == 1) {
        int len = strlen(line);
        if (iscntrl(c) || !isprint(c)) {
            switch (c) {
                case M_CERASE: {
                    // Когда вводится символ ERASE, стирается
                    // последний символ в текущей строке.

                    line[len - 1] = 0;

                    // [D - Move cursor left one char
                    // [K - Clear line from cursor right
                    printf("\33[D\33[K");

                    break;
                }

                case M_CKILL: {
                    // Когда вводится символ KILL, стираются
                    // все символы в текущей строке.

                    line[0] = 0;

                    // [2K - Clear entire line
                    printf("\33[2K\r");

                    break;
                }

                case M_CWERASE: {
                    // Когда вводится CTRL-W, стирается последнее слово в текущей
                    // строке, вместе со всеми следующими за ним пробелами.

                    int word_start = 0;
                    char prev = ' ';
                    for (int i = 0; i < len; i++) {
                        if (line[i] != ' ' && prev == ' ')
                            word_start = i;
                        
                        prev = line[i];
                    }

                    line[word_start] = 0;

                    // [<n>D - Move cursor left n chars
                    // [K - Clear line from cursor right
                    printf("\33[%dD\33[K", len - word_start);
                    break;
                }

                case M_CEOF: {
                    // Программа завершается, когда введен CTRL-D
                    // и курсор находится в начале строки.

                    if (line[0] == 0)
                        exit(0); 
                        
                    break;
                }

                default: {
                    // Все непечатаемые символы, кроме перечисленных выше, должны
                    // издавать звуковой сигнал, выводя на терминал символ CTRL-G.
                    putchar('\a');
                    break;
                }
            }
        } 
        else {
            if (len == LINE_LENGTH) {
                putchar('\n');
                len = 0;
            }

            line[len++] = c;
            line[len] = 0;

            putchar(c);
            
            // Проверка, если слово пересекает 40-й столбец
            if (len == LINE_LENGTH && c != ' ') {
                // Ищем начало текущего слова
                int word_start = len - 1;
                while (word_start >= 0 && line[word_start] != ' ') {
                    word_start--;
                }
                word_start++; // Переходим к первому символу слова
                
                // Если слово начинается до 40-го столбца
                if (word_start < LINE_LENGTH) {
                    printf("\33[%dD\33[K", len - word_start);
                    // Переносим слово на новую строку
                    printf("\n");
                    
                    // Копируем слово в начало строки
                    int word_len = len - word_start;
                    memmove(line, line + word_start, word_len);
                    line[word_len] = 0;
                    
                    // Выводим перенесенное слово
                    for (int i = 0; i < word_len; i++) {
                        putchar(line[i]);
                    }
                    
                    // Обновляем длину
                    len = word_len;
                }
            }
        }
        fflush(NULL);
    }
    return 0;
}