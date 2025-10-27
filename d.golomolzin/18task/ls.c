#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>



int main (int argc, char* argv[]) {
    
    if (argc < 2) {
        printf("Программа: %s <path>\n", argv[0]);
        return 1;
    }

    // массив всех существующих комбинаций прав
    char* rights[8] = {
        "---", "--x", "-w-","-wx",
        "r--", "r-x", "rw-", "rwx"
    };
    
    for (int i = 1; i != argc; i++) {
    
        // так как путь абсолютный, то нужно убрать  слеши
        // возвращает указатель на местонахождение последнего совпадения с символом c в строке
        char* filename = strrchr(argv[i], '/');
        // если нету, ну и не надо
        if (filename == NULL) {
            filename = argv[i];
        }
        // иначе сдвигаем указатель на единицу
        else filename++;
    
        // информация об указанном файле
        struct stat st;
        // struct stat {
        //     dev_t         st_dev;      /* устройство */
        //     ino_t         st_ino;      /* inode */
        //     mode_t        st_mode;     /* режим доступа */
        //     nlink_t       st_nlink;    /* количество жестких ссылок */
        //     uid_t         st_uid;      /* идентификатор пользователя-владельца */
        //     gid_t         st_gid;      /* идентификатор группы-владельца */
        //     dev_t         st_rdev;     /* тип устройства */
        //                             /* (если это устройство) */
        //     off_t         st_size;     /* общий размер в байтах */
        //     blksize_t     st_blksize;  /* размер блока ввода-вывода */
        //                             /* в файловой системе */
        //     blkcnt_t      st_blocks;   /* количество выделенных блоков */
        //     time_t        st_atime;    /* время последнего доступа */
        //     time_t        st_mtime;    /* время последней модификации */
        //     time_t        st_ctime;    /* время последнего изменения */
        // };
    
        // если не получилось
        if ( stat(argv[i], &st) == -1) {
            perror("stat");
            return 1;
        }
        else {
    
            // взятие соотвествующих координат в тупую
            int own_r   = (st.st_mode >> 6) & 0b111;
            int group_r = (st.st_mode >> 3) & 0b111;
            int user_r  = st.st_mode & 0b111;

            // первый символ 'd' - директория (каталог), '-' - простой файл, иначе '?'
            char fst_bit = S_ISDIR(st.st_mode) ? 'd' : ( S_ISREG(st.st_mode) ? '-' : '?');
    
            // получаем структуру с информацией и переданном пользователе
            struct passwd* pswd = getpwuid( st.st_uid );
            // struct passwd {
            //     char    *pw_name;       /* имя пользователя */
            //     char    *pw_passwd;     /* пароль пользователя */
            //     uid_t   pw_uid;         /* id пользователя */
            //     gid_t   pw_gid;         /* id группы */
            //     char    *pw_gecos;      /* настоящее имя */
            //     char    *pw_dir;        /* домашний каталог */
            //     char    *pw_shell;      /* программа-оболочка */
            // };

            // возвращает указатель на структуру, содержащую информацию из файла /etc/group о группе, идентификатор которой совпадает с gid.
            struct group* grp = getgrgid( st.st_gid );
            // struct group {
            //     char    *gr_name;        /* название группы */
            //     char    *gr_passwd;      /* пароль группы */
            //     gid_t   gr_gid;          /* идентификатор группы */
            //     char    **gr_mem;        /* члены группы */
            // };
    
            // форматируем дату
            char time_buf[13];
            struct tm* timeinfo = localtime(&st.st_mtime);
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
    
            char* display_name = (strcmp(filename, ".") == 0) ? "." : filename;

            printf("%c%s%s%s %ld %-8s %-8s %ld %12s %s\n",
                fst_bit,                                        // d/-/?
                rights[own_r],
                rights[group_r],
                rights[user_r],                                 // права
                (long)st.st_nlink,
                pswd->pw_name,                                  // Имена собственника 
                grp->gr_name,
                st.st_size,
                time_buf,
                display_name);                                  // и группы файла
                
                // printf("%12s %s\n",
                //     time_buf,           // Дату модификации файла (используйте mtime).
                //     filename);          // Имя файла (если было задано имя с путем, нужно распечатать только имя).
                // }
                
                // // Если файл является обычным файлом, его размер. Иначе оставьте это поле пустым.
                // if (fst_bit == '-') {
                //     printf("%8ld ", st.st_size);
                }
    }

    return 0;
}