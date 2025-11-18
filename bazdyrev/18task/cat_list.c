#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>

int main(int argc, char* argv[])
{
    setlocale(LC_TIME, "");

    if(argc < 2)
    {
        printf("Недостаточно аргументов\n");
        return 0;
    }
    else
    {
        for(int i = 1; i<argc; i++ )
        {
            struct stat sb;
    
            if (stat(argv[i], &sb) == 0) {

                if(S_ISDIR(sb.st_mode))printf("d");
                else if(S_ISREG(sb.st_mode))printf("-");
                else printf("?");

                printf("%c%c%c",
                (sb.st_mode & S_IRUSR) ? 'r' : '-',
                (sb.st_mode & S_IWUSR) ? 'w' : '-',
                (sb.st_mode & S_IXUSR) ? 'x' : '-');

                printf("%c%c%c",
                (sb.st_mode & S_IRGRP) ? 'r' : '-',
                (sb.st_mode & S_IWGRP) ? 'w' : '-',
                (sb.st_mode & S_IXGRP) ? 'x' : '-');

                printf("%c%c%c ",
                (sb.st_mode & S_IROTH) ? 'r' : '-',
                (sb.st_mode & S_IWOTH) ? 'w' : '-',
                (sb.st_mode & S_IXOTH) ? 'x' : '-');

                struct passwd *pw = getpwuid(sb.st_uid);
                struct group *gr = getgrgid(sb.st_gid);
                
                printf("%ld %s %s %ld ", sb.st_nlink, pw->pw_name, gr->gr_name, sb.st_size);
                char str_time[80];
                struct tm timeinfo;
                localtime_r(&sb.st_mtime, &timeinfo);
                strftime(str_time, sizeof(str_time), "%b %d %H:%M", &timeinfo);
                printf("%s %s\n",str_time, argv[i]);
            }
        }
    }
}