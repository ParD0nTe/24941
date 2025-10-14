#include <unistd.h>
#include <stdio.h>
#include "signal.h"
#include "stdlib.h"
typedef struct line{
    int len;
    char string[255];
}Line;
FILE *fin;
Line lines[255];
int lCnt;


void timeout(){
    for(int i=0;i<lCnt;i++){
        printf("%s\n",lines[i].string);
    }
    exit(0);
}
int main(void){
    fin =fopen("input.txt","r");
    if(fin == NULL){
        printf("Wrong file");
        return 1;
    }
    char c=1;
    lCnt=0;
    while(c != '\0'){
        c='\0';
        lines[lCnt].len=0;
        while(c != '\n'){
            if((c=fgetc(fin))==EOF){
                c='\0';
                break;
            }
            lines[lCnt].string[lines[lCnt].len]=c;
            lines[lCnt].len++;
        }
        if(c != '\0'){
            lines[lCnt].len--;
        }
        lines[lCnt].string[lines[lCnt].len]='\0';
        lCnt++;
    }
    fclose(fin);

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
            printf("%s\n",lines[op].string);
        }
    }
    return 0;

}