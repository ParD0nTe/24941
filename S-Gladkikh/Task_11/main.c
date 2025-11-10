#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"
#define SUBPROCESS "cat"
#define SUBPROCESS_PATH "/bin/cat"
#define ARGS {"cat","a.txt",NULL}
int execvpe(const char *file,char *const argv[], char *const envp[]){
    execve(file,argv,envp);
    char *PATH_ptr=getenv("PATH");
    if(PATH_ptr == NULL){
        printf("Path not found??????\n");
        return -1;
    }
    char* const PATH=malloc(strlen(PATH_ptr));
    strcpy(PATH,PATH_ptr);
    char f; 
    char *end=PATH;
    char *start = end;
    char *fpath;
    do{
        end = strchr(start,':');
        if(end == NULL) break;
        *end='\0';
        fpath = malloc(strlen(start) + strlen(file) + 1);
        strcpy(fpath,start);
        strcat(fpath,"/");
        strcat(fpath,file);
        if(access(fpath,F_OK)==0){
            break;
        }
        free(fpath);
        fpath=NULL;
        end++;
        start = end;

    }while (1);
    if(fpath == NULL){
        free(PATH);
        return -1;
    }
    return execve(fpath,argv,envp);
}

int main(void){
    extern char** environ;
    char *const args[]=ARGS;
    printf("%d\n",execvpe(SUBPROCESS_PATH,args,environ));
}