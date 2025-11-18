#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>

// print -> write
int main()
{
    char line[40][41] = {0};
    int len = 0;
    int lines = 0;

    struct termios termios;
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &termios);
    new_termios = termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);

    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);

    while (1)
    {
        const char symb = (char) getchar();

        if (symb == 127) // ERASE Backspace
        {
            if (len > 0)
            {
                line[lines][len - 1] = '\0';

                printf("\r%-40s", line[lines]);
                fflush(stdout);

                len--;
            }
            else if (lines > 0)
            {
                lines--;
                len = strlen(line[lines]);

                printf("\033[F\r%-40s", line[lines]);
                fflush(stdout);
            }
        }
        else if (symb == 0x15) // KILL CTRL-U
        {
            for (int i = 0; i < len; i++)
            {
                line[lines][i] = ' ';
            }
            line[lines][0] = '\0';
            len = 0;

            printf("\r%-40s", line[lines]);
            fflush(stdout);
        }
        else if (symb == 0x17) // CTRL-W
        {
            while (len > 0 && line[lines][len - 1] == ' ')
            {
                len--;
                line[lines][len] = '\0';
            }
            while (len > 0 && line[lines][len - 1] != ' ')
            {
                len--;
                line[lines][len] = '\0';
            }

            printf("\r%-40s", line[lines]);
            fflush(stdout);
        }
        else if (symb == 0x04) // CTRL-D
        {
            if (len == 0) break;
        }
        else if (isprint(symb) || symb == '\n')
        {
            if (symb == '\n')
            {
                lines++;
                len = 0;

                printf("\r%-40s", line[lines]);
                fflush(stdout);

                continue;
            }

            if (len == 40 && line[lines][len - 1] != ' ')
            {
                char word[40];
                int idx = 0;
                while (len > 0 && line[lines][len - 1] != ' ')
                {
                    word[idx++] = line[lines][--len];
                    line[lines][len] = '\0';
                }

                lines++;
                for (int i = 0; i < idx; i++)
                {
                    len = i;
                    line[lines][len] = word[idx - 1 - i];
                }
            }

            line[lines][len++] = symb;
            line[lines][len] = '\0';

            printf("\r%-40s", line[lines]);
            fflush(stdout);
        }
        else
        {
            write(1, "\a", 1);
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios);

    return 0;
}
