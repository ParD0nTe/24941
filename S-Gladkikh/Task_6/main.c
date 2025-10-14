#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "signal.h"
#include "stdlib.h"
typedef struct line{
    long begin;
    int len;
}Line;
int descr;
Line lines[255];
int lCnt;


void timeout(){
    for(int i=0;i<lCnt;i++){
        lseek(descr,lines[i].begin,SEEK_SET);
        char buf[255];
        read(descr,buf,lines[i].len);
        buf[lines[i].len]='\0';
        printf("%s\n",buf);
    }
    exit(0);
}
int main(void){
    descr =open("input.txt",O_RDONLY);
    char c=1;
    off_t offset=0;
    lCnt=0;
    while(c != '\0'){
        c='\0';
        lines[lCnt].begin = offset;
        lines[lCnt].len=0;
        while(c != '\n'){
            if(read(descr,&c,1)==0){
                c='\0';
                break;
            }
            lines[lCnt].len++;
        }
        if(c != '\0'){
            lines[lCnt].len--;
        }
        offset = lseek(descr,0,SEEK_CUR);
        lCnt++;
    }

    signal(SIGALRM,&timeout);
    int op = 0;
    alarm(5);
    scanf("%d", &op);
    alarm(0);
    while(op != -1){
        if(op > lCnt || op < 1){
            printf("Line number out of range\n");
        }
        else{
            op--;
            lseek(descr,lines[op].begin,SEEK_SET);
            char buf[255];
            read(descr,buf,lines[op].len);
            buf[lines[op].len]='\0';
            printf("%s\n",buf);
        }
        scanf("%d", &op);
    }
    return 0;

}