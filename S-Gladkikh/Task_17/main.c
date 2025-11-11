#include "unistd.h"
#include "termios.h"
#include "stdio.h"
#include "string.h"

int lseekSpace(char *str,int size){
    char *ptr = &str[size-1];
    while(*ptr != ' ' && size != 0){
        ptr--;
        printf("\b");
        size--;
    }
    return size;
}
int lseekLit(char *str, int size){
    char *ptr = &str[size-1];
    while(*ptr == ' ' && size != 0){
        ptr--;
        printf("\b");
        size--;
    }
    return size;
}

int main(void){
    struct termios params;
    tcgetattr(0,&params);
    params.c_lflag &= ~(ICANON|ECHO);
    params.c_cc[VMIN]=1;
    params.c_cc[VTIME]=1;
    tcsetattr(0,TCSANOW,&params);
    char buf[41];
    buf[40]='\0';
    int bufSize=0;
    while (1)
    {
        char word[41];
        char c = getchar();
        int wordpos;
        const char bell = 7;
        switch(c){
            case EOF:break;
            case 4:
                if(bufSize==0) return 0;
                break;
            case 21:
                bufSize=0;
                printf("\e[40D\e[2K");
                break;
            case 23:
                bufSize=lseekLit(buf,bufSize);
                wordpos=lseekSpace(buf,bufSize);
                printf("\e[K");
                bufSize=wordpos;
                break;
            case 127:
                printf("\b\e[K");
                if(bufSize>0) bufSize --;
                break;
            case '\n':
                printf("\n");
                bufSize=0;
                break;
            default:
                if(c < 32){
                    write(1,&bell,1);
                    break;
                }
                if(bufSize == 40){
                    buf[bufSize]='\0';
                    wordpos=lseekSpace(buf,bufSize);
                    if(wordpos == 0){
                        printf("\e[40C");
                        break;
                    }
                    strcpy(word,&buf[wordpos]);
                    printf("\e[K\n");
                    bufSize=0;
                    strcpy(&buf[0],word);
                    bufSize += strlen(word);
                    printf("%s",word);
                }
                buf[bufSize]=c;
                bufSize++;
                putchar(c);
                break;
        }
    }

}