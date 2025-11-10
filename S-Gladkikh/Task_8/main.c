#include "fcntl.h"
#include "unistd.h"
#include "stdlib.h"
int main(void){
    struct flock flk;
    flk.l_type=F_WRLCK;
    flk.l_whence=SEEK_SET;
    flk.l_start=0;
    flk.l_len=0;
    int dscr=open("a.txt",O_WRONLY);
    const char *test="123";
    write(dscr,test,3);
    system("nano");
    while (1)
    {
    }
    
    return 0;
}