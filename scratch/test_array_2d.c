#include <stdio.h>
#define num_col 5
typedef int row[num_col];

void print(const row * nums, int x, int y) {
    for (int i=0; i<x; i++) {
        printf("index %d: ", i);
        for (int j=0; j<y; j++) {
            printf("%d, ", nums[i][j]);
        }
        printf("\n");
    }
}


int main() {
    row nums[] = {{1, 1, 4, 5, 1},
                  {9, 7, 6 ,9 ,3},
                  {0, 0, 6, 5, 4}};
    print(nums, (int)sizeof(nums)/sizeof(row), num_col);
    return 0;
}