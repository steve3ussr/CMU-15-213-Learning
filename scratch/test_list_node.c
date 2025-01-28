#include <stdio.h>
#include <malloc.h>

typedef struct node {
    int val;
    struct node *next;
}Node;

# define N 10
#define new_node malloc(sizeof(Node))

Node * build_list(const int num_list[N]) {
    Node *head = new_node;
    head->val = 0;
    head->next = NULL;

    Node *curr = head;
    for (int i=0; i<N; i++) {
        curr->next = new_node;
        curr = curr->next;
        curr->val = num_list[i];
        curr->next = NULL;
    }
    return head->next;

}

int main() {
    int nums[N] = {114, 514, 18, 19, 24, 99, 15, 93, 73, 765};
    Node* head = build_list(nums);

    Node* curr = head;
    int cnt = 0;
    while (curr != NULL) {
        printf("%.2d: %.4d\n", cnt, curr->val);
        cnt ++;
        curr = curr->next;
    }
}
