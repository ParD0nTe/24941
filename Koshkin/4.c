#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Sent
{
    char * s;
    struct Sent * next;
}sent;

int main(void){
    sent * M = (sent *)malloc(sizeof(sent));
    sent * m = M;
    char str[32];
    FILE *file = fopen("secret.txt", "r");
    fgets(str, 20, file);
    while (str[0]!='.')
    {
        m->s = (char *) malloc(sizeof(char) * strlen(str));
        for(int i = 0; i<strlen(str)-1; i++){
            m->s[i] = str[i];
        }
        m->next = (sent *)malloc(sizeof(sent));
        m = m->next;
        m->s = NULL;
        fgets(str, 20, file);
    }
    m = M;
    while (m->s!= NULL)
    {
        sent * help = m->next;
        printf("%s ", m->s);
        free(m->s);
        free(m);
        m = help;        
    }
    printf("\n");
    fclose(file);
}