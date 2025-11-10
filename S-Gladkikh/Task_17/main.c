#include "unistd.h"
#include "termios.h"
#include "stdio.h"
int main(void){
    struct termios params;
    tcgetattr(0,&params);
    printf("%d\n",params.c_iflag & ICANON);
    params.c_iflag &= ~ICANON;
    printf("%d\n",params.c_iflag & ICANON);
    tcsetattr(0,TCSANOW,&params);
    while (1)
    {
        /* code */
    }
    
}