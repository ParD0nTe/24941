#include <stdio.h>
#include <unistd.h>
// #include <getopt.h>
// #include <sys/resource.h>

void print_ids() {
	printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

void work_with_file() {
    FILE* file;
    file = fopen("testfile.txt", "r");
    if (file == NULL) {
        perror("Error with openning file\n");
        return;
    }
    else {
        printf("File open succesful\n");
        fclose(file);
        printf("File closed\n");
        return;
    }
}

void change_uid() {
    printf("Result of changing euif: %d\n", setuid(getuid()));
}

int main() {
    print_ids();
    work_with_file();    
    change_uid();
    print_ids();
    work_with_file();
    return 0;
}