// #include <stdio.h>
// #include <unistd.h>
// #include <termios.h>
// #include <ctype.h>
// #include <string.h>
//
// // print -> write
// int main()
// {
//     char line[40][41] = {0};
//     int len = 0;
//     int lines = 0;
//
//     struct termios termios;
//     struct termios new_termios;
//     tcgetattr(STDIN_FILENO, &termios);
//     new_termios = termios;
//     new_termios.c_lflag &= ~(ICANON | ECHO);
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);
//
//     new_termios.c_cc[VMIN] = 1;
//     new_termios.c_cc[VTIME] = 0;
//
//     while (1)
//     {
//         const char symb = (char) getchar();
//
//         if (symb == 127) // ERASE Backspace
//         {
//             if (len > 0)
//             {
//                 line[lines][len - 1] = '\0';
//
//                 char buffer[41];
//                 snprintf(buffer, sizeof(buffer), "\r%-40s", line[lines]);
//                 write(STDOUT_FILENO, buffer, strlen(buffer));
//
//                 // printf("\r%-40s", line[lines]);
//                 // fflush(stdout);
//                 len--;
//             }
//             else if (lines > 0)
//             {
//                 lines--;
//                 len = strlen(line[lines]);
//
//                 char buffer[41];
//                 snprintf(buffer, sizeof(buffer), "\033[F\r%-40s", line[lines]);
//                 write(STDOUT_FILENO, buffer, strlen(buffer));
//
//                 // printf("\033[F\r%-40s", line[lines]);
//                 // fflush(stdout);
//             }
//         }
//         else if (symb == 0x15) // KILL CTRL-U
//         {
//             for (int i = 0; i < len; i++)
//             {
//                 line[lines][i] = ' ';
//             }
//             line[lines][0] = '\0';
//             len = 0;
//
//             char buffer[41];
//             snprintf(buffer, sizeof(buffer), "\033[F\r%-40s", line[lines]);
//             write(STDOUT_FILENO, buffer, strlen(buffer));
//
//             // printf("\r%-40s", line[lines]);
//             // fflush(stdout);
//         }
//         else if (symb == 0x17) // CTRL-W
//         {
//             while (len > 0 && line[lines][len - 1] == ' ')
//             {
//                 len--;
//                 line[lines][len] = '\0';
//             }
//             while (len > 0 && line[lines][len - 1] != ' ')
//             {
//                 len--;
//                 line[lines][len] = '\0';
//             }
//
//             char buffer[41];
//             snprintf(buffer, sizeof(buffer), "\033[F\r%-40s", line[lines]);
//             write(STDOUT_FILENO, buffer, strlen(buffer));
//
//             // printf("\r%-40s", line[lines]);
//             // fflush(stdout);
//         }
//         else if (symb == 0x04) // CTRL-D
//         {
//             if (len == 0) break;
//         }
//         else if (isprint(symb) || symb == '\n')
//         {
//             if (symb == '\n')
//             {
//                 lines++;
//                 len = 0;
//
//                 char buffer[41];
//                 snprintf(buffer, sizeof(buffer), "\033[F\r%-40s", line[lines]);
//                 write(STDOUT_FILENO, buffer, strlen(buffer));
//
//                 // printf("\r%-40s", line[lines]);
//                 // fflush(stdout);
//
//                 continue;
//             }
//
//             if (len == 40 && line[lines][len - 1] != ' ')
//             {
//                 char word[40];
//                 int idx = 0;
//                 while (len > 0 && line[lines][len - 1] != ' ')
//                 {
//                     word[idx++] = line[lines][--len];
//                     line[lines][len] = '\0';
//                 }
//
//                 lines++;
//                 for (int i = 0; i < idx; i++)
//                 {
//                     len = i;
//                     line[lines][len] = word[idx - 1 - i];
//                 }
//
//                 char buffer[41];
//                 snprintf(buffer, sizeof(buffer), "\033[F\r%-40s", line[lines]);
//                 write(STDOUT_FILENO, buffer, strlen(buffer));
//
//                 // printf("\r%-40s", line[lines]);
//             }
//
//             line[lines][len++] = symb;
//             line[lines][len] = '\0';
//
//             char buffer[41];
//             snprintf(buffer, sizeof(buffer), "\033[F\r%-40s", line[lines]);
//             write(STDOUT_FILENO, buffer, strlen(buffer));
//
//             // printf("\r%-40s", line[lines]);
//             // fflush(stdout);
//         }
//         else
//         {
//             write(1, "\a", 1);
//         }
//     }
//
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios);
//
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#define MAX_LEN 40
#define BELL "\a"
#define BACKSPACE "\b \b"

struct termios original_settings;

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
}

void setup_terminal(void) {
    struct termios new;
    tcgetattr(STDIN_FILENO, &original_settings);
    atexit(restore_terminal);
    new = original_settings;
    new.c_lflag &= ~(ICANON | ECHO);
    new.c_cc[VMIN] = 1;
    new.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
}

void find_last_space(char *str, int len, int *last_space) {
    *last_space = -1;
    for (int i = len - 1; i >= 0; i--) {
        if (str[i] == ' ') {
            *last_space = i;
            break;
        }
    }
}

void handle_word_erase(char *str, int *len, int *last_space) {
    while (*len > 0 && str[*len - 1] == ' ') {
        write(STDOUT_FILENO, BACKSPACE, 3);
        (*len)--;
    }
    while (*len > 0 && str[*len - 1] != ' ') {
        write(STDOUT_FILENO, BACKSPACE, 3);
        (*len)--;
    }
    str[*len] = '\0';
    find_last_space(str, *len, last_space);
}

void wrap_line(char *str, int *len, int *last_space, char ch) {
    if (ch == ' ') {
        write(STDOUT_FILENO, "\n ", 2);
        *len = 1;
        str[0] = ch;
        *last_space = 0;
    } else if (*last_space != -1) {
        int move_len = *len - *last_space - 1;

        write(STDOUT_FILENO, "\n", 1);
        write(STDOUT_FILENO, str + *last_space + 1, move_len);
        write(STDOUT_FILENO, &ch, 1);

        memmove(str, str + *last_space + 1, move_len);
        str[move_len] = ch;
        *len = move_len + 1;

        find_last_space(str, *len, last_space);
    } else {
        write(STDOUT_FILENO, "\n", 1);
        write(STDOUT_FILENO, &ch, 1);
        *len = 1;
        str[0] = ch;
        *last_space = -1;
    }
}

int main() {
    setup_terminal();

    char str[MAX_LEN + 1] = {0};
    int len = 0, last_space = -1;
    char ch;

    while (read(STDIN_FILENO, &ch, 1) > 0) {
        switch (ch) {
            case 127: // Backspace
                if (len > 0) {
                    write(STDOUT_FILENO, BACKSPACE, 3);
                    str[--len] = '\0';
                    if (len == last_space) find_last_space(str, len, &last_space);
                }
                break;

            case 21: // Ctrl+U (Kill)
                while (len > 0) {
                    write(STDOUT_FILENO, BACKSPACE, 3);
                    len--;
                }
                str[0] = '\0';
                last_space = -1;
                break;

            case 4: // Ctrl+D
                if (len == 0) {
                    write(STDOUT_FILENO, "\n", 1);
                    return 0;
                }
                break;

            case 23: // Ctrl+W
                if (len > 0) handle_word_erase(str, &len, &last_space);
                break;

            case 10: // Enter
                write(STDOUT_FILENO, "\n", 1);
                len = 0;
                last_space = -1;
                str[0] = '\0';
                break;

            default:
                if (ch < 32 || ch == 127) {
                    write(STDOUT_FILENO, BELL, 1);
                } else if (len < MAX_LEN) {
                    str[len++] = ch;
                    write(STDOUT_FILENO, &ch, 1);
                    if (ch == ' ') last_space = len - 1;
                } else {
                    wrap_line(str, &len, &last_space, ch);
                }
                break;
        }
    }

    write(STDOUT_FILENO, "\n", 1);
    return 0;
}
