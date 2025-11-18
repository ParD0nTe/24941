#include <sys/stat.h>
#include "stdio.h"
#include <unistd.h>
#include <sys/types.h>
#include "pwd.h"
#include "grp.h"
#include "time.h"

void print_permissions(mode_t mode) {
    printf("%c%c%c%c%c%c%c%c%c",
        (mode & S_IRUSR) ? 'r' : '-',
        (mode & S_IWUSR) ? 'w' : '-',
        (mode & S_IXUSR) ? 'x' : '-',
        (mode & S_IRGRP) ? 'r' : '-',
        (mode & S_IWGRP) ? 'w' : '-',
        (mode & S_IXGRP) ? 'x' : '-',
        (mode & S_IROTH) ? 'r' : '-',
        (mode & S_IWOTH) ? 'w' : '-',
        (mode & S_IXOTH) ? 'x' : '-');
}
char *getname(char *str){
    int i=0;
    for(;str[i] != 0;i++);
    for(;i>0&&str[i]!='/';i--);
    return str + i;
    
}
int main(int argc,char **argv){
    for(int i=1;i<argc;i++){
        struct stat st;
        stat(argv[i],&st);
        if(S_ISDIR(st.st_mode)){
            printf("d ");
        }
        else if(S_ISREG(st.st_mode)){
            printf("- ");
        }
        else{
            printf("? ");
        }
        print_permissions(st.st_mode);
        printf("%s ",getpwuid(st.st_uid)->pw_name);
        printf("%s ",getgrgid(st.st_gid)->gr_name);
        if(S_ISREG(st.st_mode)){
            printf("%ld ",st.st_size);
        }
        else{
            printf("* ");
        }
        char *tstring =ctime(&st.st_mtime);
        int j=0;
        for(;tstring[j+1]!=0;j++);
        
        tstring[j]=0;

        printf("%s ",tstring);
        printf("%s\n",getname(argv[i]));
        
    }
}
