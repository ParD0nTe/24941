#include <iostream>
#include <cstdlib>
#include <cstring>

struct Node {
    int len;
    char* value;
    Node* next;
};

void freeall(Node* root){
    if (root->next != NULL)
        freeall(root->next);
    free(root->value);
    free(root);
}

int main() {
    char* buffer;
    Node* root = new Node;
    Node* last = root;
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL){
            break;
        }
        if (buffer[0] == '.')
            break;
        Node* newn = new Node;
        newn->len = strlen(buffer);
        newn->value = (char*)malloc(newn->len+1);
        strcpy(newn->value, buffer);
        newn->next = NULL;
        last->next = newn;
        last = last->next;
    }
    last = root->next;
    while (last != NULL) {
        printf("%s", last->value);
        last = last->next;
    }
    freeall(root);
    return 0;
}