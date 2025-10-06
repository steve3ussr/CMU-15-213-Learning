#include <stdio.h>
#include "csapp.h"
#include <netdb.h>


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE  1049000
#define MAX_OBJECT_SIZE 102400
#define uint            unsigned int
#define MAX_URL         1024

/* You won't lose style points for including this long line in your code */
static char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void forward(int);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int parse_req_line(char *req_line, char *method, char *url, char *version, int fd){
    /* 
        fd
            1. NULL: parse only
            2. int : verify method, version, call clienterror
        
        return value: 
            0: no error
            1: error 
    */

    // TOO LONG request line
    if (strlen(req_line) >= 1024){
        if (fd) { clienterror(fd, 
                              req_line, 
                              "414", 
                              "URI Too Long", 
                              "The URI provided was too long for the proxy to process");}
        return 1;
    }

    // MALFORMED request line
    if (sscanf(req_line, "%s %s %s\r\n", method, url, version) != 3){
        if (fd) { clienterror(fd, 
                              method, 
                              "400", 
                              "Bad Request", 
                              "Proxy cannot process the client's request due to a client-side error");}
        return 1;}

    // for debug
        printf("\t[Request Line   \t] %s", req_line);
    printf("\t[Request Line: Method \t] %s\n", method);
    printf("\t[Request Line: URL    \t] %s\n", url);
    printf("\t[Request Line: Version\t] %s\n", version);
    
    // MALFORMED method
    if (strcasecmp(method, "GET")){
        if (fd) { clienterror(fd, 
                              method, 
                              "501", 
                              "Not Implemented", 
                              "Proxy does not implement this method");}
        return 1;}

    // MALFORMED version
    if (strcasecmp(version, "HTTP/1.0") && strcasecmp(version, "HTTP/1.1")){
        if (fd) { clienterror(fd, 
                              version, 
                              "505", 
                              "Not Supported", 
                              "Proxy does not support this version");}
        return 1;}
    
    return 0;
}

int parse_url(char *url, char *host, char *port, char *path, char *query, int fd){
    /* 
        fd
            1. NULL: parse only
            2. int : verify method, version, call clienterror
        
        return value: 
            0: static
            1: dyna
            -1: error 
    */

    char tmp[MAXLINE];

    // MALFORMED scheme
    if (!sscanf(url, "http://%s", tmp)) {
        if (fd) { clienterror(fd, 
                              url, 
                              "422", 
                              "Unprocessable Content", 
                              "Proxy does not support this scheme");}
        return -1;}

    char *p_colon = strchr(tmp, ':');
    char *p_slash = strchr(tmp, '/');
    
    // MALFORMED path
    if ((!p_slash) || (p_colon > p_slash)) {  // no path || colon in path
        if (fd) { clienterror(fd, 
                              url, 
                              "422", 
                              "Unprocessable Content", 
                              "Proxy cannot parse path");}
        return -1;}

    // detach host, port, path_query(tmp)
    *p_slash = '\0';
    if (p_colon){
        *p_colon = '\0';
        strcpy(host, tmp);
        strcpy(tmp, p_colon+1);     // tmp: 15213/cgi-bin/adder?1&2

        strcpy(port, tmp);
        strcpy(tmp, p_slash+1);}    // tmp: cgi-bin/adder?1&2

    else{
        strcpy(host, tmp);
        strcpy(port, "80");
        strcpy(tmp, p_slash+1);}    // tmp: cgi-bin/adder?1&2

    // MALFORMED port
    for (char *p=port; *p!='\0'; p++){
        if (!isdigit(*p)) {
            if (fd) { clienterror(fd, 
                                  url, 
                                  "422", 
                                  "Unprocessable Content", 
                                  "Malformed port");}
            return -1;}}
    int port_int = atoi(port);
    if (port_int > 65535){
        if (fd) { clienterror(fd, 
                              url, 
                              "422", 
                              "Unprocessable Content", 
                              "Malformed port");}
        return -1;}

    // MALFORMED host
    if (strchr(host, '?')){
        if (fd) { clienterror(fd, 
                              url, 
                              "422", 
                              "Unprocessable Content", 
                              "Malformed host");}
        return -1;}

    // detach path, query from path_query(tmp)
    char *p_quest = strchr(tmp, '?');
    int is_dyna = 0;
    strcpy(path, "/"); strcpy(query, "");

    if (p_quest) { *p_quest = '\0'; strcpy(query, p_quest+1); is_dyna=1;}
    strcat(path, tmp);
    if (path[strlen(path)-1] == '/') {strcat(path, "home.html");}

    // UNSAFE path
    if (strstr(path, "..")) {
        if (fd) { clienterror(fd, 
                              url, 
                              "422", 
                              "Unprocessable Content", 
                              "malicious cotnent huh?");}
        return -1;
    }


    // debug print
    printf("\t[Request Line: host   \t] %s\n", host);
    printf("\t[Request Line: port   \t] %s\n",port);
    printf("\t[Request Line: path   \t] %s\n", path);
    printf("\t[Request Line: query  \t] %s\n", query);

    return is_dyna;
}

uint is_prefix(char *prefix, char *string){
    return strncmp(prefix, string, strlen(prefix))==0;
}

int check_header(char *entry) {
    /*

    return value    description                              what u should do in caller
       -1           ERROR                                    stop directly
        0           no custom header entry is found          APPEND
        1           client req a custom Host                 APPEND
        2           client req a custom User-Agent           DROP
        4           client req a custom Connection           DROP
        8           client req a custom Proxy-Connection     DROP
    */
    printf("\t[Request Headers] %s", entry);

    if (is_prefix("Host: ", entry))                     {return 1;}
    else if (is_prefix("User-Agent: ", entry))          {*entry='\0'; return 0;}
    else if (is_prefix("Connection: ", entry))          {*entry='\0'; return 0;}
    else if (is_prefix("Proxy-Connection: ", entry))    {*entry='\0'; return 0;}
    else                                                { return 0;}
}

struct Msg {
    char msg[MAXLINE];
    int nleft;
} proxy_forw, proxy_reply;

int append_msg (struct Msg *msg_p, char *content){
    int n = strlen(content);
    if (n > msg_p->nleft) {return 1;}

    strcat(msg_p->msg, content);
    msg_p->nleft -= n;
    return 0;
}

void forward(int client_fd)
{
    // READ request line
    rio_t rio_client; Rio_readinitb(&rio_client, client_fd);
    char buf[MAXLINE];
    if (!Rio_readlineb(&rio_client, buf, MAXLINE)) return;


    // PARSE request line
    char method[MAX_URL], url[MAX_URL], version[MAX_URL], host[MAX_URL], port[MAX_URL], path[MAX_URL], query[MAX_URL];
    if (parse_req_line(buf, method, url, version, client_fd)) {return;}
    int is_dyna; 
    if ((is_dyna=parse_url(url, host, port, path, query, client_fd)) == -1) {return;}


    // INITIAL Proxy-forw, MAKE request line
    proxy_forw.nleft = MAXLINE-1;
    if (is_dyna)    sprintf(buf, "GET %s?%s HTTP/1.0\r\n", path, query);
    else            sprintf(buf, "GET %s HTTP/1.0\r\n", path);
    if (append_msg(&proxy_forw, buf)) return;

    // MAKE Proxy-forw headers
    int flag_host_hdr = 0;  // 4-bits: Proxy-Connection
    int tmp;

    printf(" -- REQ HEADERS BEGIN -- \n"); 
    Rio_readlineb(&rio_client, buf, MAXLINE);               // first line
    flag_host_hdr = check_header(buf);                      // first line
    if ((tmp = append_msg(&proxy_forw, buf)) < 0) return;   // first line

    while (1) {
        Rio_readlineb(&rio_client, buf, MAXLINE);               // other lines
        if (!strcmp(buf, "\r\n")) break;
        flag_host_hdr = check_header(buf);                      // other lines
        if ((tmp = append_msg(&proxy_forw, buf)) < 0) return;   // other lines
    }

    if (!flag_host_hdr){
        sprintf(buf, "Host: %s:%s\r\n", host, port);
        if ((tmp = append_msg(&proxy_forw, buf)) < 0) return;
    }

    if ((tmp = append_msg(&proxy_forw, user_agent_hdr)) < 0) return;
    if ((tmp = append_msg(&proxy_forw, "Connection: Close\r\n")) < 0) return;
    if ((tmp = append_msg(&proxy_forw, "Proxy-Connection: Close\r\n")) < 0) return;
    if ((tmp = append_msg(&proxy_forw, "\r\n")) < 0) return;    // ending
    printf(" -- REQ HEADERS  END  -- \n");  


    // PREVIEW Proxy-forw
    printf("ready to send Proxy-forw. \n\n--------PREVIEW START ------\n");
    printf("%s", proxy_forw.msg);
    printf("--------PREVIEW END --------\n\n");


    // connect to real server
    int server_fd = Open_clientfd(host, port); 
    rio_t rio_server; Rio_readinitb(&rio_server, server_fd);
    printf("Connected to server (%s:%s)\n", host, port);


    // SEND Proxy-forw
    Rio_writen(server_fd, proxy_forw.msg, strlen(proxy_forw.msg));
    strcpy(proxy_forw.msg, "");


    // GET Proxy-reply headers
    int content_length;
    while (Rio_readlineb(&rio_server, buf, MAXLINE)){

        // get Content-length
        if (is_prefix("Content-length: ", buf)){
            content_length = atoi(buf+16);
            printf("content-length is %d\n", content_length);}
        // stop after receiving empty line
        Rio_writen(client_fd, buf, strlen(buf));
        if (!strcmp(buf, "\r\n")) break;  // check if is \r\n
    }
    strcpy(buf, "");
    

    // GET Proxy-reply entity
    // while (content_length) {
    //     tmp = Rio_readnb(&rio_server, buf, 1000);
    //     Rio_writen(client_fd, buf, tmp);
    //     content_length -= tmp;
    // }

    char *srcp = Mmap(0, content_length, PROT_READ, MAP_PRIVATE, server_fd, 0); //line:netp:servestatic:mmap
    Rio_writen(client_fd, srcp, content_length);     //line:netp:servestatic:write
    Munmap(srcp, content_length);             //line:netp:servestatic:munmap


    // Teardown
    Close(server_fd);
}

int main(int argc, char **argv)
{
    // Step 1: check argument 
    if (argc != 2)
    {
        fprintf(stderr, "Proxy usage: %s <proxy_port_number>\n", argv[0]);
        exit(1);
    }

    // Step 2: listen, accept and forward
    int listenfd = Open_listenfd(argv[1]);
    socklen_t clientlen;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    int connfd;
    struct sockaddr_storage clientaddr;
    while (1) 
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, 
                    client_hostname, MAXLINE, 
                    client_port, MAXLINE, 
                    NI_NUMERICHOST);
        printf("Accept a client (%s:%s)\n", client_hostname, client_port);
        forward(connfd);
        Close(connfd);
    }


    printf("%s", user_agent_hdr);
    return 0;
}

void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
