int mul2(int x , int y) {return x+y;};

void multstore(int x, int y, int *res) {
    int _res = mul2(x, y);
    *res = _res+1;
}
