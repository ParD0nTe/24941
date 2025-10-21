#include <iostream>
#include <cstring>
#include <stdio.h>
#include <map>
#include <ctime>
#include <chrono>
#include <thread>
#include <limits>
#include <string>

using namespace std;

void separate_file(const char* filename, map<int, string>& mp) {
    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error via open file\n");
        return;
    }
    int slen = 100;
    int i = 0;
    mp[i] = "";
    char buffer[100];
    int n = 0;
    while (n = fread(buffer, 1, 100, file)) {
        for (int j=0;j<n;j++) {
            if (buffer[j] == '\n') {
                mp[++i] = "";
            }
            else {
                mp[i] += buffer[j];
            }
        }
        if (n < slen)
            break;
    }
    fclose(file);
}

int main() {
    map<int, string> mp;
    separate_file("ttext.txt", mp);
    clock_t start = clock(); 
    int x;
    cout << "Enter string number: ";
    while ((clock() - start) / CLOCKS_PER_SEC < 5) {
        if (cin >> x) {
            break;
        }
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    if ((clock() - start) / CLOCKS_PER_SEC >= 5) {
        cout << "Time end\n" << endl;
        for (auto [key, value] : mp) {
            cout << key << ' ' << value.length() << value << endl;
        }
    } else {
        cout << mp[x].length() << ' ' << mp[x] << endl;
    }
    return 0;
}