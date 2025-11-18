#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>

// Структура для хранения информации о строке
typedef struct {
    off_t offset; // Отступ (позиция начала строки в файле)
    size_t length; // Длина строки (включая \n)
} LineInfo;

// Функция для вывода всего содержимого файла
void print_whole_file(int fd) {
    char buffer[1024];
    ssize_t bytes_read;

    // Перемещаемся в начало файла
    lseek(fd, 0, SEEK_SET);

    // Читаем и выводим файл
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
}

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

    // Проверяем, есть ли строки в файле
    if (line_count == 0) {
        printf("Ошибка: файл пустой или не содержит строк.\n");
        free(lines);
        close(fd);
        return 1;
    }

    // Запрашиваем номер строки с таймаутом
    int line_number;
    while (1) {
        printf("Введите номер строки (0 для выхода, 5 секунд на ввод): ");
        fflush(stdout); // Сбрасываем буфер вывода

        // Настраиваем таймаут с помощью select
        fd_set read_fds;
        struct timeval timeout;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        timeout.tv_sec = 5; // 5 секунд
        timeout.tv_usec = 0;

        int ready = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == 0) {
            // Таймаут: ввод не был осуществлен в течение 5 секунд
            printf("\nВремя вышло! Выводим содержимое файла:\n");
            print_whole_file(fd);
            free(lines);
            close(fd);
            return 0;
        }

        // Ввод доступен, читаем номер строки
        scanf("%d", &line_number);
        while (getchar() != '\n'); // Очищаем остаток строки

        if (line_number == 0) {
            break; // Выход при вводе 0
        }

        if (line_number < 1 || line_number > (int)line_count) {
            printf("Ошибка: строка %d не существует. Всего строк: %zu\n", line_number, line_count);
            continue;
        }

        // Перемещаемся на начало строки
        off_t offset = lines[line_number - 1].offset;
        size_t length = lines[line_number - 1].length;

        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("Ошибка позиционирования");
            continue;
        }

        // Читаем строку
        char *line_buffer = (char *)malloc(length + 1); // +1 для \0
        if (!line_buffer) {
            perror("Ошибка выделения памяти");
            continue;
        }

        bytes_read = read(fd, line_buffer, length);
        if (bytes_read != (ssize_t)length) {
            printf("Отладка: прочитано %zd байт, ожидалось %zu\n", bytes_read, length);
            free(line_buffer);
            continue;
        }

        line_buffer[length] = '\0'; // Добавляем завершающий нуль для printf
        printf("Строка %d: %s", line_number, line_buffer);
        free(line_buffer);
    }

    // Освобождаем ресурсы
    free(lines);
    close(fd);
    return 0;
}