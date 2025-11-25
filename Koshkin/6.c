#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>

// Узел связного списка для хранения информации о строке
typedef struct LineNode {
    off_t offset;           // Отступ (позиция начала строки в файле)
    size_t length;          // Длина строки
    struct LineNode *next;  // Указатель на следующий узел
} LineNode;

// Функция для вывода всего содержимого файла
void print_whole_file(int fd) {
    char buffer[1024];
    ssize_t bytes_read;

    lseek(fd, 0, SEEK_SET);
    printf("\nВремя вышло! Выводим содержимое файла:\n");
    
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
}

int main() {
    int fd;
    char buffer[1];
    ssize_t bytes_read;
    
    // Инициализация связного списка
    LineNode *head = NULL;
    LineNode *tail = NULL;
    LineNode *current_node = NULL;
    
    size_t line_count = 0;
    off_t current_offset = 0;

    // Открываем файл
    fd = open("secret.txt", O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return 1;
    }

    // Запоминаем начальную позицию
    current_offset = lseek(fd, 0L, SEEK_CUR);

    // Читаем файл посимвольно и строим связный список
    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (current_node == NULL) {
            // Начало новой строки - создаем новый узел
            current_node = (LineNode *)malloc(sizeof(LineNode));
            if (!current_node) {
                perror("Ошибка выделения памяти");
                close(fd);
                return 1;
            }
            current_node->offset = current_offset;
            current_node->length = 0;
            current_node->next = NULL;
            
            // Добавляем узел в список
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
        
        if (buffer[0] == '\n') {
            // Завершаем текущую строку
            current_node = NULL;
            current_offset = lseek(fd, 0L, SEEK_CUR);
        }
    }

    // Выводим таблицу для отладки
    printf("Таблица отступов и длин строк:\n");
    LineNode *temp = head;
    size_t i = 1;
    while (temp != NULL) {
        printf("Строка %zu: отступ = %ld, длина = %zu\n", i, temp->offset, temp->length);
        temp = temp->next;
        i++;
    }

    // Запрашиваем номер строки с таймаутом
    int line_number;
    while (1) {
        printf("Введите номер строки (0 для выхода, 5 секунд на ввод): ");
        fflush(stdout);

        // Настраиваем таймаут с помощью select
        fd_set read_fds;
        struct timeval timeout;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int ready = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == 0) {
            // Таймаут
            print_whole_file(fd);
            break;
        }

        // Ввод доступен, читаем номер строки
        scanf("%d", &line_number);
        while (getchar() != '\n'); // Очищаем остаток строки

        if (line_number == 0) {
            break;
        }

        if (line_number < 1 || line_number > (int)line_count) {
            printf("Ошибка: строка %d не существует. Всего строк: %zu\n", line_number, line_count);
            continue;
        }

        // Находим нужный узел в списке
        temp = head;
        for (int j = 1; j < line_number; j++) {
            temp = temp->next;
        }

        // Перемещаемся на начало строки и читаем ее
        if (lseek(fd, temp->offset, SEEK_SET) == -1) {
            perror("Ошибка позиционирования");
            continue;
        }

        char *line_buffer = (char *)malloc(temp->length + 1);
        if (!line_buffer) {
            perror("Ошибка выделения памяти");
            continue;
        }

        bytes_read = read(fd, line_buffer, temp->length);
        if (bytes_read != (ssize_t)temp->length) {
            printf("Отладка: прочитано %zd байт, ожидалось %zu\n", bytes_read, temp->length);
        }

        line_buffer[temp->length] = '\0';
        printf("Строка %d: %s", line_number, line_buffer);
        free(line_buffer);
    }

    // Освобождаем память (очищаем связный список)
    temp = head;
    while (temp != NULL) {
        LineNode *to_free = temp;
        temp = temp->next;
        free(to_free);
    }

    close(fd);
    return 0;
}