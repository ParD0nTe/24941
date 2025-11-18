#define _XOPEN_SOURCE 700
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// функция преобразует bиты прав доступа в символьную строку(чтобы м ывидели именно rws)
static void perms_string(mode_t m, char out[10]) {
    out[0] = (m & S_IRUSR) ? 'r' : '-';
    out[1] = (m & S_IWUSR) ? 'w' : '-';
    out[2] = (m & S_IXUSR) ? 'x' : '-';
    out[3] = (m & S_IRGRP) ? 'r' : '-';
    out[4] = (m & S_IWGRP) ? 'w' : '-';
    out[5] = (m & S_IXGRP) ? 'x' : '-';
    out[6] = (m & S_IROTH) ? 'r' : '-';
    out[7] = (m & S_IWOTH) ? 'w' : '-';
    out[8] = (m & S_IXOTH) ? 'x' : '-';
}

// о первому символу выясняем тип файла
static char type_char(mode_t m) {
    if (S_ISDIR(m)) return 'd'; // каталог
    if (S_ISREG(m))  return '-'; // файл
    return '?'; //прочее
}

// дата модиифкации файла через mtime
static void format_mtime(time_t t, char *buf, size_t sz) {
    struct tm lt; // структура ханит время в разложенном виде
    localtime_r(&t, &lt); // ункция переводит time_t в struct tm
    // простой формат: 2025-11-10 09:42
    strftime(buf, sz, "%Y-%m-%d %H:%M", &lt);
}

// возвращает читаемое имя владельца файла (логин пользователя) по его UID
// если имя найти нельзя — возвращает числовой UID в виде строки
static const char* owner_name(uid_t uid, char *buf, size_t sz) {
    struct passwd *pw = getpwuid(uid);
    if (pw)
        return pw->pw_name;
    snprintf(buf, sz, "%u", (unsigned)uid);// записывает строку не в консоль, а в буфер, с ограничением длины
    return buf;
}

static const char* group_name(gid_t gid, char *buf, size_t sz) {
    struct group *gr = getgrgid(gid);
    if (gr) return gr->gr_name;
    snprintf(buf, sz, "%u", (unsigned)gid);
    return buf;
}

// возвращает имя файла без пути
static const char* base_name(const char *path, char *tmp, size_t sz) {
    // tmp для basename, тк она меняет строку
    strncpy(tmp, path, sz - 1);
    tmp[sz - 1] = '\0';
    return basename(tmp);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s PATH [PATH2 ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        const char *path = argv[i];
        struct stat st;
        // lstat считывает информацию о файле
        if (lstat(path, &st) != 0) {
            fprintf(stderr, "Error: %s: %s\n", path, strerror(errno));
            continue;
        }

        char perms[10];
        perms_string(st.st_mode, perms);
        perms[9] = '\0';

        char tchar = type_char(st.st_mode);

        char owbuf[32], grbuf[32];
        const char *owner = owner_name(st.st_uid, owbuf, sizeof owbuf);
        const char *group = group_name(st.st_gid, grbuf, sizeof grbuf);

        char datebuf[32];
        format_mtime(st.st_mtime, datebuf, sizeof datebuf);

        char namebuf[1024];
        const char *fname = base_name(path, namebuf, sizeof namebuf);

        char sizebuf[16];
        if (S_ISREG(st.st_mode)) {
            snprintf(sizebuf, sizeof sizebuf, "%lld", (long long)st.st_size);
        } else {
            memset(sizebuf, ' ', 10);
            sizebuf[10] = '\0';
        }

        // формат строк:
        // [тип][права] [links] [owner] [group] [size(или пробелы)] [date] [name]
        // Ширины подогнаны для аккуратной таблицы
        printf("%c%s %3lu %-10s %-10s %10s %s %s\n",
               tchar, perms,
               (unsigned long)st.st_nlink,
               owner,
               group,
               sizebuf,
               datebuf,
               fname);
    }

    return 0;
}
