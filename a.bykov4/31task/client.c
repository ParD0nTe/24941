#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#define SOCKET_PATH "task31_socket"

int main() {
    struct timespec tstart, tnow;
    clock_gettime(CLOCK_MONOTONIC, &tstart);

    int iter = 0;

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &tnow);
        double dt =
            (tnow.tv_sec - tstart.tv_sec) +
            (tnow.tv_nsec - tstart.tv_nsec) / 1e9;

        if (dt >= 5.0)
            break;

        int cid = (iter % 3) + 1;

        pid_t cpid = fork();
        if (cpid == 0) {

            /* === INLINED send_messages === */

            int cfd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (cfd == -1) exit(0);

            struct sockaddr_un caddr = { .sun_family = AF_UNIX };
            strncpy(caddr.sun_path, SOCKET_PATH, sizeof(caddr.sun_path) - 1);

            if (connect(cfd, (struct sockaddr *)&caddr, sizeof(caddr)) == -1) {
                close(cfd);
                exit(0);
            }

            struct timespec tt;
            clock_gettime(CLOCK_MONOTONIC, &tt);
            double tcur = tt.tv_sec + tt.tv_nsec / 1e9;

            char outbuf[256];
            snprintf(outbuf, sizeof(outbuf),
                     "Client%d message: Hello at %.3f sec!\n",
                     cid, tcur);

            write(cfd, outbuf, strlen(outbuf));
            close(cfd);

            exit(0);

            /* === END INLINED send_messages === */

        } else if (cpid > 0) {
            iter++;
        } else {
            perror("fork");
            break;
        }
    }

    int st;
    while (waitpid(-1, &st, WNOHANG) > 0) {
        /* cleanup */
    }

    return 0;
}
