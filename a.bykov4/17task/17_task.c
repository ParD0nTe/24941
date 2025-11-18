#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>

#define LINE_LIMIT        40
#define BUFFER_LIMIT      (LINE_LIMIT * 10)

#define KEY_ERASE   0x7F
#define KEY_CTRL_U  0x15
#define KEY_CTRL_W  0x17
#define KEY_CTRL_D  0x04
#define KEY_CTRL_G  0x07
#define KEY_CTRL_A  0x01

#define CUR_UP      "\033[A"
#define CUR_RIGHT   "\033[C"
#define SOUND       "\a"
#define ERASE_OUT   "\b \b"

static char input_buffer[BUFFER_LIMIT + 1];

static int current_line = 0;     // текущая строка
static size_t cursor_pos = 0;    // положение курсора в общем буфере

static struct termios saved_termios;

/* Восстановить исходный режим терминала */
void reset_terminal_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);
}

/* Настроить терминал в raw-подобный режим */
void init_terminal_mode() {
    struct termios temp;

    if (tcgetattr(STDIN_FILENO, &saved_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    atexit(reset_terminal_mode);

    temp = saved_termios;
    temp.c_lflag &= ~(ICANON | ECHO);
    temp.c_cc[VMIN] = 1;
    temp.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &temp) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

int main() {

    init_terminal_mode();

    printf("ERASE - Стирает последний символ\n");
    printf("CTRL+U - Стирает текущую строку\n");
    printf("CTRL+W - Стирает последнее слово\n");
    printf("CTRL+A - Стирает все строки\n");
    printf("CTRL+D - Завершение (если в начале)\n");
    printf("Максимальная длина строки и слова: 40 символов\n");
    printf("Ввод:\n");

    char ch;
    ssize_t r;

    while (1) {

        r = read(STDIN_FILENO, &ch, 1);
        if (r <= 0) continue;

        /* Завершение (CTRL+D в начале) */
        if (ch == KEY_CTRL_D) {
            if (cursor_pos == 0) {
                printf("Работа завершена\n");
                break;
            }
            continue;
        }

        /* ERASE */
        else if (ch == KEY_ERASE) {

            if (cursor_pos == 0)
                continue;

            int at_line_start = (cursor_pos % LINE_LIMIT == 0);

            cursor_pos--;

            if (at_line_start && current_line > 0) {
                current_line--;
                write(1, CUR_UP, 3);
                for (int i = 0; i < LINE_LIMIT; i++)
                    write(1, CUR_RIGHT, 3);
                write(1, ERASE_OUT, 3);
            }
            else {
                write(1, ERASE_OUT, 3);
            }
        }

        /* CTRL+A — стереть всё */
        else if (ch == KEY_CTRL_A) {

            if (cursor_pos > 0) {

                for (int i = 0; i <= current_line; i++) {
                    write(1, "\r", 1);
                    for (int j = 0; j < LINE_LIMIT; j++)
                        write(1, " ", 1);
                    write(1, "\r", 1);
                    if (i < current_line) write(1, CUR_UP, 3);
                }

                cursor_pos = 0;
                current_line = 0;
            }
        }

        /* CTRL+U — стереть текущую строку */
        else if (ch == KEY_CTRL_U) {

            if (cursor_pos == 0) continue;

            int line_start = (cursor_pos / LINE_LIMIT) * LINE_LIMIT;
            int to_clear = cursor_pos - line_start;

            for (int i = 0; i < to_clear; i++)
                write(1, ERASE_OUT, 3);

            for (int i = line_start; i < cursor_pos; i++)
                input_buffer[i] = '\0';

            cursor_pos = line_start;

            if (to_clear == LINE_LIMIT && current_line > 0)
                current_line--;
        }

        /* CTRL+W — удалить последнее слово */
        else if (ch == KEY_CTRL_W) {

            int in_spaces = 0;

            while (cursor_pos > 0) {

                cursor_pos--;

                if (isspace(input_buffer[cursor_pos])) {
                    if (in_spaces == 0)
                        in_spaces = 1;
                } else {
                    if (in_spaces == 1) {
                        cursor_pos++;
                        break;
                    }
                }

                if (cursor_pos % LINE_LIMIT == LINE_LIMIT - 1
                    && current_line > 0) {

                    current_line--;
                    write(1, CUR_UP, 3);
                    write(1, "\r", 1);

                    for (int i = 0; i < LINE_LIMIT; i++)
                        write(1, CUR_RIGHT, 3);

                    write(1, ERASE_OUT, 3);
                }
                else {
                    write(1, ERASE_OUT, 3);
                }
            }
        }

        /* Печатные символы */
        else if (ch >= 32 && ch <= 126) {

            if (cursor_pos < BUFFER_LIMIT) {

                input_buffer[cursor_pos++] = ch;
                write(1, &ch, 1);

                if (cursor_pos % LINE_LIMIT == 0) {
                    int in_spaces = 0;
                    int word_idx = 0;
                    char word[40];
                    cursor_pos--;
                    while (cursor_pos > 0) {
                        word[word_idx++] = input_buffer[cursor_pos--];
                        
                        if (word_idx == 39){
                            return 1;
                        }

                        if (isspace(input_buffer[cursor_pos])) {
                            if (in_spaces == 0)
                                in_spaces = 1;
                        } else {
                            if (in_spaces == 1) {
                                cursor_pos++;
                                break;
                            }
                        }

                        if (cursor_pos % LINE_LIMIT == LINE_LIMIT - 1
                            && current_line > 0) {

                            current_line--;
                            write(1, CUR_UP, 3);
                            write(1, "\r", 1);

                            for (int i = 0; i < LINE_LIMIT; i++)
                                write(1, CUR_RIGHT, 3);

                            write(1, ERASE_OUT, 3);
                        }
                        else {
                            write(1, ERASE_OUT, 3);
                        }
                    }
                    
                    while (cursor_pos % LINE_LIMIT != 0){
                        input_buffer[cursor_pos++] = " ";
                        write(1, " ", 1);
                    }

                    write(1, "\n", 1);
                    current_line++;
                    for (int i = word_idx - 1; i >= 0; i--){
                        ch = word[i];
                        input_buffer[cursor_pos++] = ch;
                        write(1, &ch, 1);
                    }
                }

            }
            else {
                write(1, SOUND, 1);
            }
        }

        /* Все прочие непечатные — звуковой сигнал */
        else {
            write(1, SOUND, 1);
        }
    }

    return 0;
}

