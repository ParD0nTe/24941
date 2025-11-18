#include <stdio.h>
#include <time.h>

int main() {
    time_t now;
    struct tm *tp;

    time(&now);
    tp = gmtime(&now);
    tp->tm_hour -= 8;
    mktime(tp);

    printf("//- Время в Калифорнии: %02d:%02d / День: %02d/%02d -//\n",
           tp->tm_hour, tp->tm_min, tp->tm_mday, tp->tm_mon+1);
}
