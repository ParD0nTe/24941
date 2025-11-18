#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>

// Структура для строки
typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

// Вывод всего файла
void print_whole_file(const char *mapped, off_t file_size) {
    if (file_size > 0) {
        printf("%.*s", (int)file_size, mapped);
    }
    if (file_size == 0 || mapped[file_size - 1] != '\n') {
        putchar('\n');
    }
}

int main() {
    int fd = open("secret.txt", O_RDONLY);

    struct stat sb;
    fstat(fd, &sb);

    off_t file_size = sb.st_size;

    char *mapped = mmap(NULL, (size_t)file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    // === Строим таблицу строк ===
    size_t capacity = 16;  // начальная ёмкость
    size_t line_count = 0;
    LineInfo *lines = malloc(capacity * sizeof(LineInfo));

    off_t pos = 0, line_start = 0;
    while (pos < file_size) {
        if (mapped[pos] == '\n') {
            if (line_count >= capacity) {
                capacity *= 2;
                LineInfo *temp = realloc(lines, capacity * sizeof(LineInfo));
                lines = temp;
            }
            lines[line_count].offset = line_start;
            lines[line_count].length = (size_t)(pos - line_start + 1);
            line_count++;
            line_start = pos + 1;
        }
        pos++;
    }

    // Последняя строка без \n
    if (line_start < file_size) {
        if (line_count >= capacity) {
            capacity *= 2;
            LineInfo *temp = realloc(lines, capacity * sizeof(LineInfo));
            lines = temp;
        }
        lines[line_count].offset = line_start;
        lines[line_count].length = (size_t)(file_size - line_start);
        line_count++;
    }

    // === Отладка: таблица ===
    printf("Таблица отступов и длин строк:\n");
    for (size_t i = 0; i < line_count; i++) {
        printf("Строка %zu: отступ = %ld, длина = %zu\n",
               i + 1, lines[i].offset, lines[i].length);
    }

    if (line_count == 0) {
        printf("Ошибка: файл не содержит строк.\n");
        free(lines);
        munmap(mapped, file_size);
        close(fd);
        return EXIT_FAILURE;
    }

    // === Интерактивный ввод с таймаутом ===
    int line_number;
    while (1) {
        printf("Введите номер строки (0 для выхода, 5 секунд на ввод): ");
        fflush(stdout);

        fd_set fds;
        struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ret == 0) {
            printf("\nВремя вышло! Выводим содержимое файла:\n");
            print_whole_file(mapped, file_size);
            break;
        }

        scanf("%d", &line_number);

        if (line_number == 0) break;
        if (line_number < 1 || (size_t)line_number > line_count) {
            printf("Ошибка: строка %d не существует. Всего строк: %zu\n",
                   line_number, line_count);
            continue;
        }

        size_t idx = (size_t)line_number - 1;
        const char *line_ptr = mapped + lines[idx].offset;
        size_t len = lines[idx].length;

        // Убираем \n в конце, если есть
        size_t print_len = len;
        if (print_len > 0 && line_ptr[print_len - 1] == '\n') {
            print_len--;
        }

        printf("Строка %d: %.*s\n", line_number, (int)print_len, line_ptr);
    }

    // === Освобождение ===
    free(lines);
    munmap(mapped, file_size);
    return 0;
}