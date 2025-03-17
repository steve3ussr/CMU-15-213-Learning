/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

void solve_non_diagonal_v1(int base_Ax, int base_Ay, int M, int N, int A[N][M], int B[M][N]){
    for(int i=base_Ax; i<base_Ax+8; i++){
        for(int j=base_Ay; j<base_Ay+8; j++){
            B[j][i] = A[i][j];
        }
    }
}

void solve_non_diagonal_v2(int base_Ax, int base_Ay, int M, int N, int A[N][M], int B[M][N]){
    for(int i=base_Ax; i<base_Ax+4; i++){
        for(int j=base_Ay; j<base_Ay+4; j++){
            B[j][i] = A[i][j];}}
    for(int i=base_Ax; i<base_Ax+4; i++){
        for(int j=base_Ay+4; j<base_Ay+8; j++){
            B[j][i] = A[i][j];}}
    for(int i=base_Ax+4; i<base_Ax+8; i++){
        for(int j=base_Ay+4; j<base_Ay+8; j++){
            B[j][i] = A[i][j];}}
    for(int i=base_Ax+4; i<base_Ax+8; i++){
        for(int j=base_Ay; j<base_Ay+4; j++){
            B[j][i] = A[i][j];}}
}

void solve_non_diagonal_v3(int base_Ax, int base_Ay, int M, int N, int A[N][M], int B[M][N]){
    for(int i=0; i<4; i++){
        for (int j=0; j<4; j++){
            B[base_Ay+j][base_Ax+i] = A[base_Ax+i][base_Ay+j];
            B[base_Ay+j][base_Ax+i+4] = A[base_Ax+i][base_Ay+j+4];}}

    int t1, t2, t3, t4;
    for(int i=0; i<4; i++){
        t1=B[base_Ay+i][base_Ax+4]; t2=B[base_Ay+i][base_Ax+5]; t3=B[base_Ay+i][base_Ax+6]; t4=B[base_Ay+i][base_Ax+7];
        for(int j=4; j<8; j++){
            B[base_Ay+i][base_Ax+j] = A[base_Ax+j][base_Ay+i];}
        B[base_Ay+i+4][base_Ax+0] = t1;
        B[base_Ay+i+4][base_Ax+1] = t2;
        B[base_Ay+i+4][base_Ax+2] = t3;
        B[base_Ay+i+4][base_Ax+3] = t4;
    }

    for(int i=4; i<8; i++){
        for (int j=4; j<8; j++){
            B[base_Ay+j][base_Ax+i] = A[base_Ax+i][base_Ay+j];}}

}

void solve_diagonal_v1(int base_x, int base_y, int M, int N, int A[N][M], int B[M][N]){
    for(int i=0; i<4; i++){
        for (int j=4; j<8; j++){
            B[base_x+j][base_y+i] = A[base_x+i][base_y+j];}}

    int t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = A[base_x+2][base_y+0]; t2 = A[base_x+2][base_y+1]; t3 = A[base_x+2][base_y+2]; t4 = A[base_x+2][base_y+3];
    t5 = A[base_x+3][base_y+0]; t6 = A[base_x+3][base_y+1]; t7 = A[base_x+3][base_y+2]; t8 = A[base_x+3][base_y+3];

    B[base_x+2][base_y+0] = A[base_x+0][base_y+2]; B[base_x+2][base_y+1] = A[base_x+1][base_y+2];
    B[base_x+3][base_y+0] = A[base_x+0][base_y+3]; B[base_x+3][base_y+1] = A[base_x+1][base_y+3];

    B[base_x+2][base_y+2] = t3; B[base_x+2][base_y+3] = t7;
    B[base_x+3][base_y+2] = t4; B[base_x+3][base_y+3] = t8;

    t3 = A[base_x+0][base_y+0]; t4 = A[base_x+0][base_y+1];
    t7 = A[base_x+1][base_y+0]; t8 = A[base_x+1][base_y+1];

    B[base_x+0][base_y+0]=t3; B[base_x+0][base_y+1]=t7; B[base_x+0][base_y+2]=t1; B[base_x+0][base_y+3]=t5;
    B[base_x+1][base_y+0]=t4; B[base_x+1][base_y+1]=t8; B[base_x+1][base_y+2]=t2; B[base_x+1][base_y+3]=t6;

    for(int i=4; i<8; i++){
        for(int j=0; j<4; j++){
            B[base_x+j][base_y+i] = A[base_x+i][base_y+j];}}

    t1=A[base_x+6][base_y+4]; t2=A[base_x+6][base_y+5]; t3=A[base_x+6][base_y+6]; t4=A[base_x+6][base_y+7];
    t5=A[base_x+7][base_y+4]; t6=A[base_x+7][base_y+5]; t7=A[base_x+7][base_y+6]; t8=A[base_x+7][base_y+7];

    B[base_x+6][base_y+4] = A[base_x+4][base_y+6]; B[base_x+6][base_y+5] = A[base_x+5][base_y+6]; B[base_x+6][base_y+6] = t3; B[base_x+6][base_y+7] = t7;
    B[base_x+7][base_y+4] = A[base_x+4][base_y+7]; B[base_x+7][base_y+5] = A[base_x+5][base_y+7]; B[base_x+7][base_y+6] = t4; B[base_x+7][base_y+7] = t8;

    t3=A[base_x+4][base_y+4]; t4=A[base_x+4][base_y+5];
    t7=A[base_x+5][base_y+4]; t8=A[base_x+5][base_y+5];

    B[base_x+4][base_y+4]=t3; B[base_x+4][base_y+5]=t7; B[base_x+4][base_y+6]=t1; B[base_x+4][base_y+7]=t5;
    B[base_x+5][base_y+4]=t4; B[base_x+5][base_y+5]=t8; B[base_x+5][base_y+6]=t2; B[base_x+5][base_y+7]=t6;
}

void solve_diagonal_v2(int base_x, int base_y, int M, int N, int A[N][M], int B[M][N]){
    int t1, t2, t3, t4, t5, t6, t7, t8;

    t1=A[base_x+0][base_y+6]; t2=A[base_x+0][base_y+7];
    t3=A[base_x+1][base_y+6]; t4=A[base_x+1][base_y+7];
    t5=A[base_x+2][base_y+6]; t6=A[base_x+2][base_y+7];
    t7=A[base_x+3][base_y+6]; t8=A[base_x+3][base_y+7];

    B[base_x+6][base_y+0]=t1; B[base_x+6][base_y+1]=t3; B[base_x+6][base_y+2]=t5; B[base_x+6][base_y+3]=t7;
    B[base_x+7][base_y+0]=t2; B[base_x+7][base_y+1]=t4; B[base_x+7][base_y+2]=t6; B[base_x+7][base_y+3]=t8;

    t1=A[base_x+0][base_y+4]; t2=A[base_x+0][base_y+5];
    t3=A[base_x+1][base_y+4]; t4=A[base_x+1][base_y+5];
    t5=A[base_x+2][base_y+4]; t6=A[base_x+2][base_y+5];
    t7=A[base_x+3][base_y+4]; t8=A[base_x+3][base_y+5];

    B[base_x+4][base_y+0]=t1; B[base_x+4][base_y+1]=t3; B[base_x+4][base_y+2]=t5; B[base_x+4][base_y+3]=t7;
    B[base_x+5][base_y+0]=t2; B[base_x+5][base_y+1]=t4; B[base_x+5][base_y+2]=t6; B[base_x+5][base_y+3]=t8;

    t1=A[base_x+0][base_y+2]; t2=A[base_x+0][base_y+3];
    t3=A[base_x+1][base_y+2]; t4=A[base_x+1][base_y+3];
    t5=A[base_x+2][base_y+2]; t6=A[base_x+2][base_y+3];
    t7=A[base_x+3][base_y+2]; t8=A[base_x+3][base_y+3];

    B[base_x+2][base_y+0]=t1; B[base_x+2][base_y+1]=t3; B[base_x+2][base_y+2]=t5; B[base_x+2][base_y+3]=t7;
    B[base_x+3][base_y+0]=t2; B[base_x+3][base_y+1]=t4; B[base_x+3][base_y+2]=t6; B[base_x+3][base_y+3]=t8;

    t1=A[base_x+0][base_y+0]; t2=A[base_x+0][base_y+1];
    t3=A[base_x+1][base_y+0]; t4=A[base_x+1][base_y+1];
    t5=A[base_x+2][base_y+0]; t6=A[base_x+2][base_y+1];
    t7=A[base_x+3][base_y+0]; t8=A[base_x+3][base_y+1];

    B[base_x+0][base_y+0]=t1; B[base_x+0][base_y+1]=t3; B[base_x+0][base_y+2]=t5; B[base_x+0][base_y+3]=t7;
    B[base_x+1][base_y+0]=t2; B[base_x+1][base_y+1]=t4; B[base_x+1][base_y+2]=t6; B[base_x+1][base_y+3]=t8;

    t1=A[base_x+4][base_y+6]; t2=A[base_x+4][base_y+7];
    t3=A[base_x+5][base_y+6]; t4=A[base_x+5][base_y+7];
    t5=A[base_x+6][base_y+6]; t6=A[base_x+6][base_y+7];
    t7=A[base_x+7][base_y+6]; t8=A[base_x+7][base_y+7];

    B[base_x+6][base_y+4]=t1; B[base_x+6][base_y+5]=t3; B[base_x+6][base_y+6]=t5; B[base_x+6][base_y+7]=t7;
    B[base_x+7][base_y+4]=t2; B[base_x+7][base_y+5]=t4; B[base_x+7][base_y+6]=t6; B[base_x+7][base_y+7]=t8;

    t1=A[base_x+4][base_y+4]; t2=A[base_x+4][base_y+5];
    t3=A[base_x+5][base_y+4]; t4=A[base_x+5][base_y+5];
    t5=A[base_x+6][base_y+4]; t6=A[base_x+6][base_y+5];
    t7=A[base_x+7][base_y+4]; t8=A[base_x+7][base_y+5];

    B[base_x+4][base_y+4]=t1; B[base_x+4][base_y+5]=t3; B[base_x+4][base_y+6]=t5; B[base_x+4][base_y+7]=t7;
    B[base_x+5][base_y+4]=t2; B[base_x+5][base_y+5]=t4; B[base_x+5][base_y+6]=t6; B[base_x+5][base_y+7]=t8;

    t1=A[base_x+4][base_y+2]; t2=A[base_x+4][base_y+3];
    t3=A[base_x+5][base_y+2]; t4=A[base_x+5][base_y+3];
    t5=A[base_x+6][base_y+2]; t6=A[base_x+6][base_y+3];
    t7=A[base_x+7][base_y+2]; t8=A[base_x+7][base_y+3];

    B[base_x+2][base_y+4]=t1; B[base_x+2][base_y+5]=t3; B[base_x+2][base_y+6]=t5; B[base_x+2][base_y+7]=t7;
    B[base_x+3][base_y+4]=t2; B[base_x+3][base_y+5]=t4; B[base_x+3][base_y+6]=t6; B[base_x+3][base_y+7]=t8;

    t1=A[base_x+4][base_y+0]; t2=A[base_x+4][base_y+1];
    t3=A[base_x+5][base_y+0]; t4=A[base_x+5][base_y+1];
    t5=A[base_x+6][base_y+0]; t6=A[base_x+6][base_y+1];
    t7=A[base_x+7][base_y+0]; t8=A[base_x+7][base_y+1];

    B[base_x+0][base_y+4]=t1; B[base_x+0][base_y+5]=t3; B[base_x+0][base_y+6]=t5; B[base_x+0][base_y+7]=t7;
    B[base_x+1][base_y+4]=t2; B[base_x+1][base_y+5]=t4; B[base_x+1][base_y+6]=t6; B[base_x+1][base_y+7]=t8;

}

void solve_diagonal_v3(int base_x, int base_y, int M, int N, int A[N][M], int B[M][N]){
    int t1, t2, t3, t4, t5, t6, t7, t8;

    B[base_x+2][base_y+0]=A[base_x+0][base_y+2]; B[base_x+2][base_y+1]=A[base_x+1][base_y+2];
    B[base_x+3][base_y+0]=A[base_x+0][base_y+3]; B[base_x+3][base_y+1]=A[base_x+1][base_y+3];
    B[base_x+6][base_y+0]=A[base_x+0][base_y+6]; B[base_x+6][base_y+1]=A[base_x+1][base_y+6];
    B[base_x+7][base_y+0]=A[base_x+0][base_y+7]; B[base_x+7][base_y+1]=A[base_x+1][base_y+7];

    t1=A[base_x+0][base_y+0]; t2=A[base_x+0][base_y+1]; t3=A[base_x+0][base_y+4]; t4=A[base_x+0][base_y+5];
    t5=A[base_x+1][base_y+0]; t6=A[base_x+1][base_y+1]; t7=A[base_x+1][base_y+4]; t8=A[base_x+1][base_y+5];

    B[base_x+4][base_y+2]=A[base_x+2][base_y+4]; B[base_x+4][base_y+3]=A[base_x+3][base_y+4];
    B[base_x+5][base_y+2]=A[base_x+2][base_y+5]; B[base_x+5][base_y+3]=A[base_x+3][base_y+5];
    B[base_x+4][base_y+0]=t3;      B[base_x+4][base_y+1]=t7;
    B[base_x+5][base_y+0]=t4;      B[base_x+5][base_y+1]=t8;
    B[base_x+0][base_y+2]=A[base_x+2][base_y+0]; B[base_x+0][base_y+3]=A[base_x+3][base_y+0];
    B[base_x+1][base_y+2]=A[base_x+2][base_y+1]; B[base_x+1][base_y+3]=A[base_x+3][base_y+1];
    B[base_x+0][base_y+0]=t1;      B[base_x+0][base_y+1]=t5;
    B[base_x+1][base_y+0]=t2;      B[base_x+1][base_y+1]=t6;

    t1=A[base_x+2][base_y+2]; t2=A[base_x+2][base_y+3]; t3=A[base_x+2][base_y+6]; t4=A[base_x+2][base_y+7];
    t5=A[base_x+3][base_y+2]; t6=A[base_x+3][base_y+3]; t7=A[base_x+3][base_y+6]; t8=A[base_x+3][base_y+7];

    B[base_x+2][base_y+2]=t1; B[base_x+2][base_y+3]=t5; B[base_x+2][base_y+4]=A[base_x+4][base_y+2]; B[base_x+2][base_y+5]=A[base_x+5][base_y+2];
    B[base_x+3][base_y+2]=t2; B[base_x+3][base_y+3]=t6; B[base_x+3][base_y+4]=A[base_x+4][base_y+3]; B[base_x+3][base_y+5]=A[base_x+5][base_y+3];
    B[base_x+6][base_y+4]=A[base_x+4][base_y+6]; B[base_x+6][base_y+5]=A[base_x+5][base_y+6]; B[base_x+6][base_y+2]=t3; B[base_x+6][base_y+3]=t7;
    B[base_x+7][base_y+4]=A[base_x+4][base_y+7]; B[base_x+7][base_y+5]=A[base_x+5][base_y+7]; B[base_x+7][base_y+2]=t4; B[base_x+7][base_y+3]=t8;

    t1=A[base_x+4][base_y+0]; t2=A[base_x+4][base_y+1]; t3=A[base_x+4][base_y+4]; t4=A[base_x+4][base_y+5];
    t5=A[base_x+5][base_y+0]; t6=A[base_x+5][base_y+1]; t7=A[base_x+5][base_y+4]; t8=A[base_x+5][base_y+5];

    B[base_x+4][base_y+4]=t3; B[base_x+4][base_y+5]=t7; B[base_x+4][base_y+6]=A[base_x+6][base_y+4]; B[base_x+4][base_y+7]=A[base_x+7][base_y+4];
    B[base_x+5][base_y+4]=t4; B[base_x+5][base_y+5]=t8; B[base_x+5][base_y+6]=A[base_x+6][base_y+5]; B[base_x+5][base_y+7]=A[base_x+7][base_y+5];
    B[base_x+0][base_y+4]=t1; B[base_x+0][base_y+5]=t5; B[base_x+0][base_y+6]=A[base_x+6][base_y+0]; B[base_x+0][base_y+7]=A[base_x+7][base_y+0];
    B[base_x+1][base_y+4]=t2; B[base_x+1][base_y+5]=t6; B[base_x+1][base_y+6]=A[base_x+6][base_y+1]; B[base_x+1][base_y+7]=A[base_x+7][base_y+1];

    t1=A[base_x+6][base_y+2]; t2=A[base_x+6][base_y+3]; t3=A[base_x+6][base_y+6]; t4=A[base_x+6][base_y+7];
    t5=A[base_x+7][base_y+2]; t6=A[base_x+7][base_y+3]; t7=A[base_x+7][base_y+6]; t8=A[base_x+7][base_y+7];
    B[base_x+2][base_y+6]=t1; B[base_x+2][base_y+7]=t5;
    B[base_x+3][base_y+6]=t2; B[base_x+3][base_y+7]=t6;
    B[base_x+6][base_y+6]=t3; B[base_x+6][base_y+7]=t7;
    B[base_x+7][base_y+6]=t4; B[base_x+7][base_y+7]=t8;

}

void solve_diagonal_v4(int base_x, int base_y, int M, int N, int A[N][M], int B[M][N]){
    int t1, t2, t3, t4, t5, t6, t7, t8;
    B[base_x+2][base_y+0] = A[base_x+0][base_y+2]; B[base_x+2][base_y+1] = A[base_x+1][base_y+2]; B[base_x+2][base_y+2] = A[base_x+0][base_y+0]; B[base_x+2][base_y+3] = A[base_x+0][base_y+1];
    B[base_x+3][base_y+0] = A[base_x+0][base_y+3]; B[base_x+3][base_y+1] = A[base_x+1][base_y+3]; B[base_x+3][base_y+2] = A[base_x+1][base_y+0]; B[base_x+3][base_y+3] = A[base_x+1][base_y+1];

    B[base_x+6][base_y+0] = A[base_x+0][base_y+6]; B[base_x+6][base_y+1] = A[base_x+1][base_y+6]; B[base_x+6][base_y+2] = A[base_x+0][base_y+4]; B[base_x+6][base_y+3]=A[base_x+0][base_y+5];  // P error
    B[base_x+7][base_y+0] = A[base_x+0][base_y+7]; B[base_x+7][base_y+1] = A[base_x+1][base_y+7]; B[base_x+7][base_y+2] = A[base_x+1][base_y+4]; B[base_x+7][base_y+3] = A[base_x+1][base_y+5]; // P error

    B[base_x+6][base_y+4] = A[base_x+4][base_y+6]; B[base_x+6][base_y+5] = A[base_x+5][base_y+6]; B[base_x+6][base_y+6] = A[base_x+4][base_y+4]; B[base_x+6][base_y+7] = A[base_x+4][base_y+5]; // P error
    B[base_x+7][base_y+4] = A[base_x+4][base_y+7]; B[base_x+7][base_y+5] = A[base_x+5][base_y+7]; B[base_x+7][base_y+6] = A[base_x+5][base_y+4]; B[base_x+7][base_y+7] = A[base_x+5][base_y+5]; // P error

    t1=A[base_x+4][base_y+0]; t2=A[base_x+4][base_y+1]; t3=A[base_x+4][base_y+2]; t4=A[base_x+4][base_y+3];
    t5=A[base_x+5][base_y+0]; t6=A[base_x+5][base_y+1]; t7=A[base_x+5][base_y+2]; t8=A[base_x+5][base_y+3];

    B[base_x+0][base_y+2] = A[base_x+2][base_y+0]; B[base_x+0][base_y+3] = A[base_x+3][base_y+0]; B[base_x+0][base_y+4] = t1; B[base_x+0][base_y+5] = t5;
    B[base_x+1][base_y+2] = A[base_x+2][base_y+1]; B[base_x+1][base_y+3] = A[base_x+3][base_y+1]; B[base_x+1][base_y+4] = t2; B[base_x+1][base_y+5] = t6;
    B[base_x+0][base_y+0] = A[base_x+2][base_y+2]; B[base_x+0][base_y+1] = A[base_x+2][base_y+3]; B[base_x+0][base_y+6] = t3; B[base_x+0][base_y+7] = t4; // Total error
    B[base_x+1][base_y+0] = A[base_x+3][base_y+2]; B[base_x+1][base_y+1] = A[base_x+3][base_y+3]; B[base_x+1][base_y+6] = t7; B[base_x+1][base_y+7] = t8; // Total error

    t1=A[base_x+2][base_y+4]; t2=A[base_x+2][base_y+5]; t3=A[base_x+2][base_y+6]; t4=A[base_x+2][base_y+7];
    t5=A[base_x+3][base_y+4]; t6=A[base_x+3][base_y+5]; t7=A[base_x+3][base_y+6]; t8=A[base_x+3][base_y+7];

    B[base_x+4][base_y+2]=t1; B[base_x+4][base_y+3]=t5; B[base_x+4][base_y+6]=A[base_x+6][base_y+4]; B[base_x+4][base_y+7]=A[base_x+7][base_y+4];
    B[base_x+5][base_y+2]=t2; B[base_x+5][base_y+3]=t6;  B[base_x+5][base_y+6]=A[base_x+6][base_y+5]; B[base_x+5][base_y+7]=A[base_x+7][base_y+5];
    B[base_x+4][base_y+4]=A[base_x+6][base_y+6]; B[base_x+4][base_y+5]=A[base_x+6][base_y+7]; // error
    B[base_x+5][base_y+4]=A[base_x+7][base_y+6]; B[base_x+5][base_y+5]=A[base_x+7][base_y+7]; // error
    B[base_x+4][base_y+0]=t3; B[base_x+4][base_y+1]=t4; // error
    B[base_x+5][base_y+0]=t7; B[base_x+5][base_y+1]=t8; // error

    t1=A[base_x+6][base_y+0]; t2=A[base_x+6][base_y+1]; t3=A[base_x+6][base_y+2]; t4=A[base_x+6][base_y+3];
    t5=A[base_x+7][base_y+0]; t6=A[base_x+7][base_y+1]; t7=A[base_x+7][base_y+2]; t8=A[base_x+7][base_y+3];

    B[base_x+2][base_y+6]=t3; B[base_x+2][base_y+7]=t7;
    B[base_x+3][base_y+6]=t4; B[base_x+3][base_y+7]=t8;

    //---------------------------------------

    B[base_x+2][base_y+4]=B[base_x+0][base_y+6]; B[base_x+2][base_y+5]=B[base_x+1][base_y+6];
    B[base_x+3][base_y+4]=B[base_x+0][base_y+7]; B[base_x+3][base_y+5]=B[base_x+1][base_y+7];
    B[base_x+0][base_y+6]=t1; B[base_x+0][base_y+7]=t5;
    B[base_x+1][base_y+6]=t2; B[base_x+1][base_y+7]=t6;
    t1=B[base_x+0][base_y+0]; t2=B[base_x+0][base_y+1]; t3=B[base_x+1][base_y+0]; t4=B[base_x+1][base_y+1];
    B[base_x+0][base_y+0]=B[base_x+2][base_y+2]; B[base_x+0][base_y+1]=B[base_x+3][base_y+2];
    B[base_x+1][base_y+0]=B[base_x+2][base_y+3]; B[base_x+1][base_y+1]=B[base_x+3][base_y+3];
    B[base_x+2][base_y+2]=t1; B[base_x+2][base_y+3]=t3;
    B[base_x+3][base_y+2]=t2; B[base_x+3][base_y+3]=t4;

    t1=B[base_x+4][base_y+0]; t2=B[base_x+4][base_y+1]; t3=B[base_x+5][base_y+0]; t4=B[base_x+5][base_y+1];
    t5=B[base_x+4][base_y+4]; t6=B[base_x+4][base_y+5]; t7=B[base_x+5][base_y+4]; t8=B[base_x+5][base_y+5];
    B[base_x+4][base_y+0]=B[base_x+6][base_y+2]; B[base_x+4][base_y+1]=B[base_x+7][base_y+2];
    B[base_x+5][base_y+0]=B[base_x+6][base_y+3]; B[base_x+5][base_y+1]=B[base_x+7][base_y+3];
    B[base_x+4][base_y+4]=B[base_x+6][base_y+6]; B[base_x+4][base_y+5]=B[base_x+7][base_y+6];
    B[base_x+5][base_y+4]=B[base_x+6][base_y+7]; B[base_x+5][base_y+5]=B[base_x+7][base_y+7];
    B[base_x+6][base_y+2]=t1; B[base_x+6][base_y+3]=t3; B[base_x+6][base_y+6]=t5; B[base_x+6][base_y+7]=t7;
    B[base_x+7][base_y+2]=t2; B[base_x+7][base_y+3]=t4; B[base_x+7][base_y+6]=t6; B[base_x+7][base_y+7]=t8;
}

void solve_global(int base_x, int base_y, int M, int N, int A[N][M], int B[M][N], int bsize){
    int maxi, maxj;
    for (int bx=0; bx<N; bx += bsize){
        for (int by=0; by<M; by+=bsize){
            maxi = bx+bsize<N?bx+bsize:N;
            maxj = by+bsize<M?by+bsize:M;
            for (int i=bx; i<maxi; i++){
                for (int j=by; j<maxj; j++){
                    B[j][i] = A[i][j];}}
        }
    }
}


char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M==8 && N==8){
        solve_diagonal_v4(0, 0, M, N, A, B);
    }
    else if(M==32 && N==32){
        solve_diagonal_v1(0, 0, M, N, A, B);
        solve_non_diagonal_v1(8, 0, M, N, A, B);
        solve_non_diagonal_v1(16, 0, M, N, A, B);
        solve_non_diagonal_v1(24, 0, M, N, A, B);

        solve_diagonal_v1(8, 8, M, N, A, B);
        solve_non_diagonal_v1(0, 8, M, N, A, B);
        solve_non_diagonal_v1(16, 8, M, N, A, B);
        solve_non_diagonal_v1(24, 8, M, N, A, B);

        solve_diagonal_v1(16, 16, M, N, A, B);
        solve_non_diagonal_v1(0, 16, M, N, A, B);
        solve_non_diagonal_v1(8, 16, M, N, A, B);
        solve_non_diagonal_v1(24, 16, M, N, A, B);

        solve_diagonal_v1(24, 24, M, N, A, B);
        solve_non_diagonal_v1(0, 24, M, N, A, B);
        solve_non_diagonal_v1(8, 24, M, N, A, B);
        solve_non_diagonal_v1(16, 24, M, N, A, B);
    }
    else if(M==64 && N==64){
        for(int i=0; i<64; i+=8){
            for(int j=0; j<64; j+=8){
                if(i==j){solve_diagonal_v4(i, j, M, N, A, B);}
                else{solve_non_diagonal_v3(i, j, M, N, A, B);}
            }
        }


    }
    else if(M==8 && N==16){
        solve_diagonal_v4(0, 0, M, N, A, B);
        solve_non_diagonal_v3(8, 0, M, N, A, B);
    }
    else {
        solve_global(0, 0, M, N, A, B, 17);
    }

}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

