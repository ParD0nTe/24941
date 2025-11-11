#include <iostream>
#include <cstring>

using namespace std;

struct sent
{
    char ** p;
    long long unsigned len = 0;
    long long unsigned capacity = 2;
};

int main(void){
    sent M;
    char str[32];
    int n = 32;
    FILE *file = fopen("secret.txt", "r");
    fgets(str, 20, file);
    M.p = (char **) malloc(sizeof(char*)*M.capacity);
    while (str[0]!='.')
    {
        char * st = (char *) malloc(sizeof(char) * strlen(str));
        for(int i = 0; i<strlen(str); i++){
            st[i] = str[i];
        }
        if(M.len != M.capacity){
            M.p[M.len] = st;
            M.len+=1;
        }else{
            char ** help = (char **) malloc(sizeof(char*)*M.capacity*2);
            for(int i = 0; i<M.capacity; i++){
                help[i] = M.p[i];
            }
            free(M.p);
            M.p = help;
            M.capacity *= 2;
            M.p[M.len] = st;
            M.len+=1;
        }
        fgets(str, 20, file);
    }
    fclose(file);
    for(int i = 0; i< M.len; i++){
        cout << M.p[i];
        free(M.p[i]);
    }
    free(M.p);
}