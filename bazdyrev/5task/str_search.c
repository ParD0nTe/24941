#include <stdio.h>
#include <fcntl.h>

int main()
{
    int str = open("input.txt", O_RDONLY);

    char buffer[1024];
    int read_bites = read(str, buffer, sizeof(buffer));

    int count_str = 0;
    for (int i = 0; i < read_bites; i++) { 
        if (buffer[i] == '\n') { 
            count_str++;
        } 
    }

    int pos_str[++count_str];
    pos_str[0] = 0;
    int idx = 1;

    for (int i = 0; i < read_bites; i++)
    {
        if (buffer[i] == '\n') 
        {
            pos_str[idx] = i + 1;
            idx++;
        }
    }

    pos_str[count_str] = read_bites;

    printf("//- Таблица -//\n");
    for (int j = 0; j < count_str; j++)
    {
        printf("%d = ", j+1);

        lseek(str, pos_str[j], 0);

        int index = pos_str[j+1] - pos_str[j];
        char output[index];
        read(str, output, index);

        for (int i = 0; i < index; i++) { printf("%c", output[i]); }
        printf("\n");
    }

    while(1)
    {
        printf("\nВведите номер строки (1-%d): ", count_str);
        int n;
        scanf("%d", &n);

        if (n == 0 || n > count_str)  { break; }

        lseek(str, pos_str[n-1], 0);

        int index = pos_str[n] - pos_str[n-1];
        char output[index];
        read(str, output, index);

        for (int i = 0; i < index; i++) {
            printf("%c", output[i]); 
        }
    }
}