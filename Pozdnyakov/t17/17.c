#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void set_terminal_mode(int mod) {
    struct termios term;
    
    tcgetattr(STDIN_FILENO, &term);
    
    if (mod) {
        term.c_lflag |= ICANON;
        term.c_lflag |= ECHO;

    } else {
        term.c_lflag &= ~ICANON;
        term.c_lflag &= ~ECHO;
        term.c_cc[VMIN] = 1; 
        term.c_cc[VTIME] = 0; 
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void add_new_str(char*input, char***arr_str, int *kol_str)
{
    if((*kol_str) == 0)
    {
        *arr_str = malloc(sizeof(char*));
        (*arr_str)[0] = input;
        (*kol_str)++;
    }
    else
    {   
        (*kol_str)++;
        *arr_str = realloc(*arr_str, *kol_str * sizeof(char*));
        (*arr_str)[*kol_str-1] = input;
    }
}

int main()
{
    struct termios original_term;

    tcgetattr(STDIN_FILENO, &original_term);

    set_terminal_mode(0);

    char ch;
    char *input = malloc(1);
    input[0] = '\0';
    int pos = 0;

    char** arr_str;
    int kol_str = 0;

    while(1)
    {
        read(STDIN_FILENO, &ch, 1);
        // printf("%d",ch);
        if(ch == 4 && pos == 0)
        {
            printf("\nВвод завершён\n");
            break;
        }
        else if(ch == 10)
        {
            pos++;
            input = realloc(input, pos*sizeof(char));
            input[pos-1] = '\0';
            printf("\n");
            fflush(stdout);

            add_new_str(input, &arr_str, &kol_str);

            pos = 0;
            input = malloc(1);
            input[0] = '\0';
        }
        else if(ch == 127 || ch == 8)
        {   
            if(pos > 0){
                pos--;
                input[pos] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        }
        else if(ch == 21)
        {
            while(pos>=1)
            {
                pos--;
                printf("\b \b");
                fflush(stdout);
                input[pos] = '\0';
            }
        }
        else if(ch == 23)
        {
            // printf("ok");
            while(pos > 0 && isspace(input[pos-1]))
            {
                input[pos-1] = '\0';
                pos--;
                printf("\b \b");
                fflush(stdout);
            }

            while(pos>0 && !isspace(input[pos-1]))
            {
                input[pos-1] = '\0';
                pos--;
                printf("\b \b");
                fflush(stdout);
            }

        }
        else if (ch >= 32 && ch <= 126) { 
            if (pos < 40) {
                pos++;
                input = realloc(input, pos*sizeof(char));
                input[pos-1] = ch;
                
                printf("%c", ch);
                fflush(stdout);
            }
            else
            {
                int i = 0;
                while(pos-i > 0 && !isspace(input[pos-i-1]))
                {
                    i++;
                }

                if(i == 40)
                {
                    for(int j = 0; j < pos; j++)
                    {
                        printf("\b \b");
                    }
                    fflush(stdout);
                    printf("\nСлишком длиное слово, строка обнулена\n");
                    fflush(stdout);
                    pos = 0;
                    free(input);
                    input = malloc(1);
                    input[0] = '\0';
                }
                else
                {
                    char *new_str = malloc((i+1) * sizeof(char));
                    for(int j = 0; j < i; j++){
                        new_str[j] = input[pos-i+j];
                        input[pos-i+j] = '\0';
                        printf("\b \b");
                    }
                    new_str[i] = ch;
                    add_new_str(input, &arr_str, &kol_str);
                    input = new_str;
                    printf("\n");
                    fflush(stdout);
                    for(int j = 0; j<i+1; j++)
                    {
                        printf("%c",input[j]);
                    }
                    fflush(stdout);
                    pos = i+1;
                }
            }
        }
        else
        {
            set_terminal_mode(1);
            printf("\a");
            fflush(stdout);
            set_terminal_mode(0);
        }
    }

    for (int i = 0; i < kol_str; i++) {
        free(arr_str[i]);
    }
    if(kol_str != 0)free(arr_str);
    free(input);

    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
    return 0;
}