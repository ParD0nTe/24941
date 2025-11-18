#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

// Глобальный массив сочетаний прав
char* rights[8] = {
        "---", "--x", "-w-","-wx",
        "r--", "r-x", "rw-", "rwx"
    };


int main (int argc, char* argv[]) {
    
    if (argc < 2) {
        printf("Недостаточно аргументов\n");
        return 1;
    }
    
    
    for (int i = 1; i != argc; i++) {
    
        // так как путь абсолютный, то нужно убрать  слеши
        // возвращает указатель на местонахождение последнего совпадения с символом c в строке
        char* filename = strrchr(argv[i], '/');
        if (filename == NULL) {
            filename = argv[i];
        }
        // иначе сдвигаем указатель на единицу
        else filename++;
    
        // информация об указанном файле
        struct stat st;
    
        // если не получилось
        if ( stat(argv[i], &st) == -1) {
            perror("stat");
            return 1;
        }
        else {
    
            // берем соответсвующие координаты для прав
            int own_right   = (st.st_mode >> 6) & 0b111;
            int group_right = (st.st_mode >> 3) & 0b111;
            int user_right  = st.st_mode & 0b111;

            // первый символ 'd' - директория (каталог), '-' - простой файл, иначе '?'
            char first_bit;

            if (S_ISDIR(st.st_mode)){
                first_bit = 'd';
            }
            else if (S_ISREG(st.st_mode)) {
                first_bit = '-';
            }
            else {
                first_bit = '?';
            }
    
            // получаем структуру с информацией и переданном пользователе
            struct passwd* pswd = getpwuid( st.st_uid );

            // возвращает указатель на структуру, содержащую информацию из файла /etc/group о группе, идентификатор которой совпадает с gid.
            struct group* grp = getgrgid( st.st_gid );
            // форматируем дату
            char time_buffer[13];
            struct tm* timeinfo = localtime(&st.st_mtime);
            strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", timeinfo);
            
            char* display_name;

            if (strcmp(filename, ".") == 0){
                display_name = ".";
            }
            else display_name = filename;

            printf("%c%s%s%s %4ld %-12s %-12s %7ld %12s %s\n",
                first_bit,                                        // d/-/?
                rights[own_right],
                rights[group_right],
                rights[user_right],                                 // права
                (long)st.st_nlink,
                pswd->pw_name,                                  // Имена собственника 
                grp->gr_name,
                st.st_size,
                time_buffer,
                display_name);
                }
    }
    return 0;
}