#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    const char *filename = "input.txt";

    int fd = open(filename, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return 1; }

    size_t size = st.st_size;
    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); close(fd); return 1; }

    int lines = 1;
    for (size_t i = 0; i < size; i++)
        if (data[i] == '\n') lines++;

    int pos[lines + 1];
    pos[0] = 0;
    int idx = 1;
    for (size_t i = 0; i < size; i++)
        if (data[i] == '\n')
            pos[idx++] = (int)i + 1;
    pos[lines] = (int)size;

    while (1) {
        int n;
        printf("Введите номер строки (1-%d, 0 — выход): ", lines);
        if (scanf("%d", &n) != 1 || n == 0) break;

        if (n < 1 || n > lines) {
            puts("Нет такой строки.");
            continue;
        }

        int start = pos[n - 1];
        int len = pos[n] - pos[n - 1];

        fwrite(data + start, 1, len, stdout);
    }

    munmap(data, size);
}
