#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>

// Узел связного списка
typedef struct LineNode {
    off_t offset;
    size_t length;
    struct LineNode *next;
} LineNode;

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

    // === Строим таблицу строк через связный список ===
    LineNode *head = NULL;
    LineNode *tail = NULL;
    LineNode *current_node = NULL;
    size_t line_count = 0;

    off_t pos = 0, line_start = 0;
    while (pos < file_size) {
        if (current_node == NULL) {
            // Начало новой строки
            current_node = malloc(sizeof(LineNode));
            current_node->offset = line_start;
            current_node->length = 0;
            current_node->next = NULL;
            
            // Добавляем в список
            if (head == NULL) {
                head = current_node;
                tail = current_node;
            } else {
                tail->next = current_node;
                tail = current_node;
            }
            line_count++;
        }

        current_node->length++;
        
        if (mapped[pos] == '\n') {
            // Завершаем текущую строку
            current_node = NULL;
            line_start = pos + 1;
        }
        pos++;
    }

    // === Отладка: таблица ===
    printf("Таблица отступов и длин строк:\n");
    LineNode *temp = head;
    size_t i = 1;
    while (temp != NULL) {
        printf("Строка %zu: отступ = %ld, длина = %zu\n",
               i, temp->offset, temp->length);
        temp = temp->next;
        i++;
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

        // Находим нужную строку в списке
        temp = head;
        for (int j = 1; j < line_number; j++) {
            temp = temp->next;
        }

        const char *line_ptr = mapped + temp->offset;
        size_t len = temp->length;

        // Убираем \n в конце, если есть
        size_t print_len = len;
        if (print_len > 0 && line_ptr[print_len - 1] == '\n') {
            print_len--;
        }

        printf("Строка %d: %.*s\n", line_number, (int)print_len, line_ptr);
    }

    // === Освобождение памяти ===
    temp = head;
    while (temp != NULL) {
        LineNode *to_free = temp;
        temp = temp->next;
        free(to_free);
    }
    
    munmap(mapped, file_size);
    return 0;
}