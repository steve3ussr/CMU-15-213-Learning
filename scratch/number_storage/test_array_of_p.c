#include <stdio.h>
#define num_col 5
typedef int row[num_col];

void print(row** data, int x, int y) {
    for (int i=0; i<x; i++) {
        printf("index %d: ", i);
        for (int j=0; j<y; j++) {
            printf("%d, ", (*data[i])[j]);
        }
        printf("\n");
    }
}

int main() {
    row r1 = {1, 1, 4, 5, 1};
    row r2 = {1, 8, 1, 9, 0};
    row r3 = {5, 6, 7, 8, 9};
    row *nums[] = {&r1, &r2, &r3};

    print(nums, sizeof(nums)/sizeof(row*), num_col);
    return 0;
}
