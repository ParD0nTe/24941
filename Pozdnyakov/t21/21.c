#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int main()
{
    struct termios original_term;

    tcgetattr(STDIN_FILENO, &original_term);

    struct termios term;
    
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~ICANON;
    term.c_cc[VMIN] = 1; 
    term.c_cc[VTIME] = 0; 

    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    char ch;
    int count = 0;
    while(1)
    {
        read(STDIN_FILENO, &ch, 1);
        if(ch == 7){
            count++;
            printf("\a");
            fflush(stdout);
        }
        else if(ch == 31)
        {
            printf("\nСигнал прозвучал %d\n",count);
            break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);

    return 0;
}