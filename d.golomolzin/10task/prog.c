#define _GNU_SOURCE // GNU-специфичные расширения. Для использования strdup(), getline()
#define _POSIX_C_SOURCE 200112L // Определяет версию POSIX стандарта = POSIX.1-2001. для getrlimit(), setrlimit()

#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>


// Печатает реальные и эффективные идентификаторы пользователя и группы
void i_flag() {

    printf("REAL UID: %d\n", getuid());     // id -ru
    printf("EFEC UID: %d\n", geteuid());    // id -u
    printf("REAL GID: %d\n", getgid());     // id -rg
    printf("EFEC GID: %d\n", getegid());    // id -g
}

// Процесс становится лидером группы. Подсказка: смотри setpgid(2).
void s_flag() {

    // ps -o pid,pgid,comm $$ -u $USER
    if (setpgid(0,0) == -1) {
        perror("Failed with setpgid");
        return;
    }
    else {
        printf("New PGID: %d\n", getpgid(0));
    }
}

// Печатает идентификаторы процесса, процесса-родителя и группы процессов
void p_flag() {

    // ps -o pid,ppid,pgid,comm $$ -u $USER
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());
    printf("PGID: %d\n", getpgrp());
}

// Печатает значение ulimit
void u_flag() {

    struct rlimit rlim;
    // получаем SOFT and HARD ограничения на размеры файлов
    if (getrlimit(RLIMIT_CPU, &rlim) == -1) {
        perror("Failed with getrlimit for FILE");
        return;
    }
    else {
        // размер в байтах
        printf("SOFT NPROC: %lu\n", rlim.rlim_cur); // ulimit -S -f
        printf("HARD NPROC: %lu\n", rlim.rlim_max); // ulimit -H -f
    }   
}

// Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C)
void U_flag(const char* str) {

    // получаем значения для установки
    long arg = atol(str);
    if (arg < 0) {
        perror("Wrong NPROC");
        return;
    }
    else {
        struct rlimit rlim;
        
        if (getrlimit(RLIMIT_CPU, &rlim) == -1) {
            perror("Failed with getrlimit for NPROC in U_flag");
            return;
        }
        // пробуем изменить soft
        // если пытается ввести больше, чем возможно
        if (rlim.rlim_max != RLIM_INFINITY && (rlim_t)arg > rlim.rlim_max) {
            fprintf(stderr, "Wrong SOFT NPROC");
            return;
        }

        rlim.rlim_cur = arg;
        
        if (setrlimit(RLIMIT_CPU, &rlim) == -1) {
            perror("Failed with setrlimit for NPROC in U_flag");
            return;
        }
        else {
            printf("New SOFT NPROC: %lu\n", rlim.rlim_cur); // ulimit -S -f
            printf("HARD NPROC: %lu\n", rlim.rlim_max);     // ulimit -H -f
        }
    }
}

// Печатает размер в байтах core-файла, который может быть создан.
void c_flag() {

    struct rlimit rlim;
    // получаем SOFT and HARD ограничения на размеры core файлов
    if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
        perror("Falied with getrlimit for CORE");
        return;
    }
    else {
        // размер в байтах
        printf("SOFT CORE: %lu\n", rlim.rlim_cur); // ulimit -S -c
        printf("HARD CORE: %lu\n", rlim.rlim_max); // ulimit -H -c
    }
}

// Изменяет размер core-файла
void C_flag(const char* str) {

    // получаем значения для установки
    long arg = atol(str);
    if (arg < 0) {
        perror("Wrong CORE size");
        return;
    }
    else {
        struct rlimit rlim;
        if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
            perror("Failed with setrlimit for CORE in C_flag");
            return;
        }

        if (rlim.rlim_max != RLIM_INFINITY && (rlim_t)arg > rlim.rlim_max) {
            fprintf(stderr, "Wrong SOFT CORE");
            return;
        }
        rlim.rlim_cur = arg;

        if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
            perror("Failed with settlimit for CORE in C_flag");
            return;
        }
        else {
            printf("New SOFT CORE: %lu\n", rlim.rlim_cur);
            printf("HARD CORE: %lu\n", rlim.rlim_max);
        }
    }
}

// Печатает текущую рабочую директорию
void d_flag() {

    char pwd[1024];
    // получаем текущую директорию
    if (getcwd(pwd, sizeof(pwd)) == NULL) { // pwd
        perror("Failed with getcwd");
        return;
    }
    else {
        printf("Current WD: %s\n", pwd);
    }
}

// Распечатывает переменные среды и их значения
void v_flag() {
    // подключаем глоб. переменную всех переменных=значений
    extern char **environ;
    printf("All Environments:\n");
    for (char **env = environ; *env != NULL; env++) {
        printf("  |—— %s\n", *env);
    }
}

// Вносит новую переменную в среду или изменяет значение существующей переменной.
void V_flag(const char* str) {

    // cоздаем копию для разбора
    char* copy_str = strdup(str);
    if (copy_str == NULL) {
        perror("Failed with strdup in V_flag");
        return;
    }
    
    // разделяем на name and value по '='
    char* name = strtok(copy_str, "=");
    char* value = strtok(NULL, "");
    
    if (name == NULL || value == NULL) {
        fprintf(stderr, "Invalid format. Use: name=value\n");
        free(copy_str);
        return;
    }
    
    // удалим лишние пробелы в значении. Если есть
    while (*value == ' ') {
        value++;
    }
    
    if (setenv(name, value, 1) == -1) {
        perror("Failed to set environment variable");
    }
    else {
        printf("Successfully set environment variable: %s=%s\n", name, value);
    }
    free(copy_str);
}

int main(int argc, char *argv[]) {

    printf(
        "\n[i] - Печатает реальные и эффективные идентификаторы пользователя и группы.\n"
        "[s] - Процесс становится лидером группы.\n"
        "[p] - Печатает идентификаторы процесса, процесса-родителя и группы процессов.\n"
        "[u] - Печатает значение ulimit.\n"
        "[U] <new_ulimit> -  Изменяет значение ulimit.\n"
        "[c] - Печатает размер в байтах core-файла, который может быть создан.\n"
        "[C] <size> -  Изменяет размер core-файла.\n"
        "[d] - Печатает текущую рабочую директорию.\n"
        "[v] - Распечатывает переменные среды и их значения.\n"
        "[V] <name=value> - Вносит новую переменную в среду или изменяет значение существующей переменной.\n\n");
        
    extern char* optarg;

    if (argc == 1) {
        printf("Empty arguments\n");
        return 1;
    }


    int c;
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != EOF) {

        switch (c) {

            case 'i':
                i_flag();
                break;

            case 's':
                s_flag();
                break;

            case 'p':
                p_flag();
                break;
                
            case 'u':
                u_flag();
                break;

            case 'U':
                U_flag(optarg);
                break;

            case 'c':
                c_flag();
                break;

            case 'C':
                C_flag(optarg);
                break;

            case 'd':
                d_flag();
                break;
                
            case 'v':
                v_flag();
                break;

            case 'V':
                V_flag(optarg);
                break;

            case '?':
                fprintf(stderr, "Unsupported options %c", c);
                break;
        }
    }
    return 0;
}
