#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <aio.h>

#define NUM_CLIENTS 10
#define SOCKET_PATH "./socket"
#define BLOCK_SIZE 4

typedef struct {
    int fd;
    int active;
    int num;
    struct aiocb aio;
    char buf[BLOCK_SIZE+1];
} client_t;

client_t clients[NUM_CLIENTS];
int server_fd;

void print_time_event(int num, const char *event) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts); // точное время

    struct tm tm_info;
    localtime_r(&ts.tv_sec, &tm_info);

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
    printf("[%d] %s %s.%03ld\n", num, event, buf, ts.tv_nsec / 1000000);
    fflush(stdout);
}

void start_aio(client_t *c) {
    memset(&c->aio, 0, sizeof(c->aio));
    c->aio.aio_fildes = c->fd;
    c->aio.aio_buf = c->buf;
    c->aio.aio_nbytes = BLOCK_SIZE;
    c->aio.aio_offset = 0;
    aio_read(&c->aio);
}

int main() {
    
    printf("\n┌────────────────────────────────────────┐\n");
    printf("│                 СЕРВЕР                 │\n");
    printf("└────────────────────────────────────────┘\n");

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    unlink(SOCKET_PATH);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, NUM_CLIENTS) < 0) { perror("listen"); return 1; }

    for (int i=0;i<NUM_CLIENTS;i++) clients[i].active=0;

    // делаем сервер неблокирующим
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    while(1) {
        // принимаем новых клиентов
        int fd = accept(server_fd, NULL, NULL);
        if(fd > 0){
            int idx = -1;
            for(int i=0;i<NUM_CLIENTS;i++) if(!clients[i].active){ idx=i; break; }
            if(idx >= 0){
                clients[idx].fd = fd;
                clients[idx].active = 1;
                clients[idx].num = idx+1;
                print_time_event(clients[idx].num, "START");
                fcntl(fd, F_SETFL, O_NONBLOCK);
                start_aio(&clients[idx]);
            } else close(fd);
        }

        // проверяем завершение aio
        for(int i=0;i<NUM_CLIENTS;i++){
            if(!clients[i].active) continue;
            int err = aio_error(&clients[i].aio);
            if(err == 0){
                int n = aio_return(&clients[i].aio);
                if(n > 0){
                    clients[i].buf[n] = 0;
                    for(int j=0;j<n;j++) clients[i].buf[j] = toupper((unsigned char)clients[i].buf[j]);
                    printf("%s", clients[i].buf);
                    fflush(stdout);
                    start_aio(&clients[i]); // новое асинхронное чтение
                } else if(n==0){
                    close(clients[i].fd);
                    print_time_event(clients[i].num,"END");
                    clients[i].active=0;
                }
            } else if(err != EINPROGRESS){
                perror("aio_error");
                close(clients[i].fd);
                clients[i].active=0;
            }
        }
        usleep(1000);
    }

    close(server_fd);
    return 0;
}
