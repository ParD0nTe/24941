#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void print_file_info(const char *filename) {
    struct stat st;
    
    if (lstat(filename, &st) == -1) {
        perror("lstat");
        return;
    }
    
    // Определяем тип файла
    char file_type;

    if (S_ISDIR(st.st_mode)) 
        file_type = 'd';
    else if (S_ISREG(st.st_mode)) 
        file_type = '-';
    else 
        file_type = '?';
    
    // Права доступа
    char permissions[10];
    permissions[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[9] = '\0';
    
    // Получаем имя владельца и группы
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    
    char owner_name[32];
    char group_name[32];
    
    if (pw != NULL) 
        strncpy(owner_name, pw->pw_name, 31);
    else 
        snprintf(owner_name, 32, "%d", st.st_uid);
    
    
    if (gr != NULL) 
        strncpy(group_name, gr->gr_name, 31);
    else 
        snprintf(group_name, 32, "%d", st.st_gid);
    
    
    // Получаем время модификации
    char time_buf[64];
    struct tm *timeinfo = localtime(&st.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
    
    // Извлекаем только имя файла из пути
    const char *basename = strrchr(filename, '/');
    if (basename != NULL) 
        basename++; // Пропускаем '/'
    else 
        basename = filename;
    
    // Форматированный вывод
    if (S_ISREG(st.st_mode))
        // Для обычных файлов выводим размер
        printf("%c%s %2ld %-8s %-8s %8ld %s %s\n",
               file_type, permissions, st.st_nlink,
               owner_name, group_name, (long)st.st_size,
               time_buf, basename);
    else
        // Для необычных файлов не выводим размер
        printf("%c%s %2ld %-8s %-8s %8s %s %s\n",
               file_type, permissions, st.st_nlink,
               owner_name, group_name, "",
               time_buf, basename);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл1> [файл2 ...]\n", argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; i++)
        print_file_info(argv[i]);
    
    return 0;
}