int func(int x) {
    int res=0;
    switch (x) {
        case 1:
            res = 101; break;
        case 2:
            res = 102; break;
        case 3:
            res = 103; break;
        case 5:
            res = 105;
        case 6:
            res += 1; break;
        default:
            res = 1000;
    }
    return res;
}
