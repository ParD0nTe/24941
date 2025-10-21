#include <iostream>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <limits>
#include <string>

using namespace std;

const char filename[] = "ttext.txt";
map<int, int> mp;

void separate_file() {
    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error via open file\n");
        return;
    }
    int slen = 100;
    int i = 1;
    mp[0] = 0;
    char buffer[100];
    int n = 0;
    int total_read = 0;
    while (n = fread(buffer, 1, 100, file)) {
        for (int j=0;j<n;j++) {
            if (buffer[j] == '\n') {
                mp[i++] = total_read+1; 
            }
            total_read++;
        }
        if (n < slen)
            break;
    }
    fclose(file);
    mp[i] = total_read+1;
}

void read_str(int start) {
    if (mp.find(start+1) == mp.end() || mp.find(start) == mp.end()) {
        perror("Out of range\n");
        return;
    }
    int file = open(filename, O_RDWR);
    if (file == -1) {
        perror("Error via open file\n");
        return;
    }
    int stop = mp[start+1];
    start = mp[start];
    int n = stop-start-1;
    cout << n << ' ';
    lseek(file, start, 0);
    char* buffer = new char[n+1];
    read(file, buffer, n);
    buffer[n] = '\0';
    cout << buffer << endl;
    close(file);
    free(buffer);
}


int main() {
    separate_file();
    for (auto [key, value] : mp) {
        cout << key << ' ' << value << endl;
    }
    int x;
    cout << "Enter string number: ";
    cin >> x;
    read_str(x-1);
    return 0;
}