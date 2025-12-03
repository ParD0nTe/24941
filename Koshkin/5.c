#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// Узел связного списка для хранения информации о строке
typedef struct LineNode {
    off_t offset;           // Отступ (позиция начала строки в файле)
    size_t length;          // Длина строки
    struct LineNode *next;  // Указатель на следующий узел
} LineNode;

int main() {
    int fd;
    char buffer[1];
    ssize_t bytes_read;
    size_t current_length = 0;
    
    // Голова и хвост связного списка
    LineNode *head = NULL;
    LineNode *tail = NULL;
    LineNode *current_node = NULL;
    size_t line_count = 0;

    // Открываем файл
    fd = open("secret.txt", O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return 1;
    }

    off_t current_offset = lseek(fd, 0L, SEEK_CUR);

    // Читаем файл посимвольно и строим связный список
    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (current_node == NULL) {
            current_node = (LineNode *)malloc(sizeof(LineNode));
            current_node->offset = current_offset;
            current_node->length = 0;
            current_node->next = NULL;
            
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

    // Запрашиваем номер строки и выводим строку
    int line_number;
    while (1) {
        printf("Введите номер строки (0 для выхода): ");
        if(scanf("%d", &line_number) != 1){
            printf("неверный формат\n");
            while ((line_number = getchar()) != '\n' && line_number != EOF);
            continue;
        }

        if (line_number == 0) {
            break;
        }

        if (line_number < 1 || line_number > line_count) {
            printf("Неверный номер строки! Доступно строк: %zu\n", line_count);
            continue;
        }

        // Находим нужный узел в списке
        temp = head;
        for (int j = 1; j < line_number; j++) {
            temp = temp->next;
        }

        // Читаем и выводим строку
        lseek(fd, temp->offset, SEEK_SET);
        char *line_buffer = (char *)malloc(temp->length + 1);
        bytes_read = read(fd, line_buffer, temp->length);
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