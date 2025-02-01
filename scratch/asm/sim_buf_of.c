#include <stdio.h>
char * gets(char * s) {
    int c;
    char *dest = s;
    while ((c=getchar()) != '\n' && c != EOF)
        *dest++ = c;
    if (c == EOF && dest == s)
        return NULL;
    *dest ++ = '\0';
    return s;
}

int main() {
    char buf[4];
    gets(buf);
    puts(buf);
    return 0;
}
