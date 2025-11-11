#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// Структура для хранения информации о строке
typedef struct {
    off_t offset; // Отступ (позиция начала строки в файле)
    size_t length; // Длина строки (включая \n)
} LineInfo;

int main() {
    int fd;
    char buffer[1]; // Буфер для посимвольного чтения
    ssize_t bytes_read;
    off_t current_offset = 0;
    size_t current_length = 0;
    LineInfo *lines = NULL; // Динамический массив для таблицы
    size_t line_count = 0; // Количество строк
    size_t capacity = 10; // Начальная емкость массива

    // Открываем файл
    fd = open("secret.txt", O_RDONLY);

    // Выделяем начальную память для таблицы строк
    lines = (LineInfo *)malloc(capacity * sizeof(LineInfo));

    // Фиксируем отступ первой строки
    lines[line_count].offset = 0;

    // Читаем файл посимвольно и строим таблицу
    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        current_length++;
        if (buffer[0] == '\n') {
            // Завершаем текущую строку
            lines[line_count].length = current_length;
            line_count++;
            current_length = 0;

            // Проверяем, нужно ли увеличить массив
            if (line_count >= capacity) {
                capacity *= 2;
                LineInfo *temp = (LineInfo *)realloc(lines, capacity * sizeof(LineInfo));
                lines = temp;
            }

            // Фиксируем отступ следующей строки
            lines[line_count].offset = lseek(fd, 0L, SEEK_CUR);
        }
    }

    // Если последняя строка не заканчивается \n, учитываем ее
    if (current_length > 0) {
        lines[line_count].length = current_length;
        line_count++;
    }

    // Выводим таблицу для отладки
    printf("Таблица отступов и длин строк:\n");
    for (size_t i = 0; i < line_count; i++) {
        printf("Строка %zu: отступ = %ld, длина = %zu\n", i + 1, lines[i].offset, lines[i].length);
    }

    // Запрашиваем номер строки и выводим строку
    int line_number;
    while (1) {
        printf("Введите номер строки (0 для выхода): ");
        scanf("%d", &line_number);

        if (line_number == 0) {
            break; // Выход при вводе 0
        }

        // Перемещаемся на начало строки
        off_t offset = lines[line_number - 1].offset;
        size_t length = lines[line_number - 1].length;
        // Читаем строку
        lseek(fd, offset, SEEK_SET);
        char *line_buffer = (char *)malloc(length + 1); // +1 для \0

        bytes_read = read(fd, line_buffer, length);

        line_buffer[length] = '\0'; // Добавляем завершающий нуль для printf
        printf("Строка %d: %s", line_number, line_buffer);
        free(line_buffer);
    }

    // Освобождаем ресурсы
    free(lines);
    close(fd);
    return 0;
}