#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>


void printFileInfo(const char *path)
{
    struct stat fileInfo;
    if (stat(path, &fileInfo) == -1)
    {
        perror("\n");
        return;
    }

    char type;
    if (S_ISDIR(fileInfo.st_mode))
    {
        type = 'd';
    }
    else if (S_ISREG(fileInfo.st_mode))
    {
        type = '-';
    }
    else
    {
        type = '?';
    }

    char permissions[255];
    permissions[0] = (fileInfo.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (fileInfo.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (fileInfo.st_mode & S_IXUSR) ? 'x' : '-';

    permissions[3] = (fileInfo.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (fileInfo.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (fileInfo.st_mode & S_IXGRP) ? 'x' : '-';

    permissions[6] = (fileInfo.st_mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (fileInfo.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (fileInfo.st_mode & S_IXOTH) ? 'x' : '-';

    permissions[9] = '\0';

    const struct passwd *pw = getpwuid(fileInfo.st_uid);
    const struct group  *gr = getgrgid(fileInfo.st_gid);
    const int nlink = fileInfo.st_nlink;

    off_t size = fileInfo.st_size;
    if (type == '?') size = 0;

    char time[100];
    const struct tm *tm_info = localtime(&fileInfo.st_mtime);
    strftime(time, sizeof(time), "%b %d %H:%M", tm_info);

    const char *filename = strrchr(path, '/');
    if (filename == NULL) filename = path;
    else filename++;

    printf("%-1c%-9s@ %-1d %-17s %-6s %lld %s %s\n",
        type, permissions, nlink, pw->pw_name, gr->gr_name, size, time, filename);
}


int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }

    for (int i = 1; i < argc; i++)
    {
        printFileInfo(argv[i]);
    }

    return 0;
}