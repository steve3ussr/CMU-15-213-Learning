#include "csapp.h"

int main(int argc, char **argv)
{
    if (argc != 2){
        fprintf(stderr, "FORMAT ERROR: %s <domain name>\n", argv[0]);
        exit(1);
    }

    int return_code;
    
    struct addrinfo *res_listp, hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((return_code = getaddrinfo(argv[1], NULL, &hints, &res_listp)) != 0){
        fprintf(stderr, 
                "getaddrinfo ERROR: cannot get addr info, %s.\n", 
                gai_strerror(return_code));
        exit(1);
    }
    
    struct addrinfo *curr;
    char buf[MAXLINE];
    for (curr=res_listp; curr; curr=curr->ai_next) {
        Getnameinfo(curr->ai_addr, 
                    curr->ai_addrlen, 
                    buf, 
                    MAXLINE, 
                    NULL, 
                    0, 
                    NI_NUMERICHOST);
        printf("%s\n", buf);
    }

    Freeaddrinfo(res_listp);
    exit(0);

}