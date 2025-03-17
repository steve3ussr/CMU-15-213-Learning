#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define uint unsigned int


typedef struct Entry_{
    char method;
    uint addr;
    uint size;
}Entry;


typedef struct Line_{
    uint v;
    uint tag;
    struct Line_ * prev;
    struct Line_ * next;
}Line;


typedef struct Set_{
    uint len;
    uint limit;
    Line * dumb_head;
    Line * dumb_tail;
}Set;


int TraverseList(Set * set, uint input_t, uint input_b) {
    Line * curr = set->dumb_head->next; // pointing at the first node / dumb_tail

    // TODO: i dont know why the second way will fail
    // curr->next && curr->next->next != NULL
    // curr->next != set->dumb_tail
    while (curr->next && curr->next->next != NULL) {
        if (curr->tag == input_t && curr->v == 1) break;
        curr = curr->next;
    }  // curr->next = dumb_tail / curr is the match line

    if (curr->tag == input_t && curr->v == 1){
        // hit!
        curr->prev->next = curr->next;
        curr->next->prev = curr->prev;
        set->dumb_tail->prev->next = curr;
        curr->prev = set->dumb_tail->prev;
        curr->next = set->dumb_tail;
        set->dumb_tail->prev = curr;
        return 1;
    }

    Line *tmp = malloc(sizeof(Line));
    tmp->v = 1;
    tmp->tag = input_t;
    tmp->prev = set->dumb_tail->prev;
    set->dumb_tail->prev->next = tmp;
    tmp->next = set->dumb_tail;
    set->dumb_tail->prev = tmp;

    if (set->len < set->limit) {
        // miss... but we can just append
        set->len += 1; return 2;}
    else {
        // miss + eviction, pop head and append
        Line *head = set->dumb_head->next;
        set->dumb_head->next = head->next;
        head->next->prev = set->dumb_head;
        free(head);
        return 3;}
}


void InitCache(Set* s[], int set_num, int line_num){
    for(int i=0; i<(1<<set_num); i++){
        s[i] = malloc(sizeof(Set));
        s[i]->len = 0;
        s[i]->limit = line_num;
        s[i]->dumb_head = malloc(sizeof(Line));
        s[i]->dumb_tail = malloc(sizeof(Line));

        s[i]->dumb_head->v = 0;
        s[i]->dumb_head->tag = 0;
        s[i]->dumb_head->prev = NULL;
        s[i]->dumb_head->next = s[i]->dumb_tail;

        s[i]->dumb_tail->v = 0;
        s[i]->dumb_tail->tag = 0;
        s[i]->dumb_tail->prev = s[i]->dumb_head;
        s[i]->dumb_tail->next = NULL;}
}


int main(int argc, char *argv[])
{
    // Part 0: exit if argc length error
    if ((argc != 9) && (argc != 10))
        exit(1);


    // Part 1: load args
    int index_arg = 1;
  

    // if res==1, all args matched
    int res = strcmp(argv[index_arg], "-s")==0 &&
              strcmp(argv[index_arg+2], "-E")==0 &&
              strcmp(argv[index_arg+4], "-b")==0 &&
              strcmp(argv[index_arg+6], "-t")==0;
    if (!res) exit(1);

    int s = strtol(argv[index_arg+1], NULL, 10);
    int E = strtol(argv[index_arg+3], NULL, 10);
    int b = strtol(argv[index_arg+5], NULL, 10);
    char *t = argv[index_arg+7];


    // Part 2: load file, get line nums -> ENTRY_LENGTH
    char string[100]; // 31 is buffer size
    FILE* file = fopen(t, "r");
    if (file == NULL) {
        fprintf(stderr, "can't open %s: %s\n", t, strerror(errno));
        exit(1);
    }
    int ENTRY_LENGTH = 0;
    while (fgets(string, 100, file)) ENTRY_LENGTH += 1;
    fclose(file);


    // Part 3: load file, initial entries
    file = fopen(t, "r");
    Entry* entries[ENTRY_LENGTH];

    char method; uint tmp_=0, addr=0, size=0; int i = 0;
    while (fgets(string, 100, file)) {

        if (string[0] == 'I') {ENTRY_LENGTH -= 1; continue;}

        tmp_ = sscanf(string, " %c 7%8x,%d", &method, &addr, &size);
        if (tmp_ != 3) tmp_ = sscanf(string, " %c %x,%d", &method, &addr, &size);

        entries[i] = malloc(sizeof(Entry));
        memcpy(&(entries[i]->method), &method, sizeof(method));
        memcpy(&(entries[i]->addr), &addr, sizeof(addr));
        memcpy(&(entries[i]->size), &size, sizeof(size));
        i += 1;
    }


    // Part 4: initial cache
    Set* cache[1<<s];
    InitCache(cache, s, E);


    // Part 5: prepare mask
    uint mask_b = (1<<b)-1;
    uint mask_s = ((1<<(s+b))-1) ^ mask_b;
    uint mask_t = 0xffffffff ^ mask_s ^ mask_b;


    // Part 6: exec for each entry, record
    int hit=0, miss=0, eviction=0;
    for(int j=0; j<ENTRY_LENGTH; j++){

        // Part 6.1: get method, addr: t-s-b
        method = entries[j]->method;
        uint addr_t = (entries[j]->addr & mask_t) >> (s+b);
        uint addr_s = (entries[j]->addr & mask_s) >> b;

        // Part 6.2: print entry info when input verbose
        if (argc == 10) printf("%c %x,%d", entries[j]->method, entries[j]->addr, entries[j]->size);

        // Part 6.3: if M, access memory twice; print if has verbose
        if (method == 'M') {
            res = TraverseList(cache[addr_s], addr_t, 0);
            if (argc == 10) {
                switch (res) {
                case 1: printf("\thit"); hit += 1; break;
                case 2: printf("\tmiss"); miss += 1; break;
                case 3: printf("\tmiss eviction");miss += 1; eviction += 1; break;}}
            else {
                switch (res) {
                case 1: hit += 1; break;
                case 2: miss += 1; break;
                case 3: miss += 1; eviction += 1; break;}}
        }

        // Part 6.4: access memory; print if has verbose
        res = TraverseList(cache[addr_s], addr_t, 0);
        if (argc == 10) {
            switch (res) {
                case 1: printf("\thit\n"); hit += 1; break;
                case 2: printf("\tmiss\n"); miss += 1; break;
                case 3: printf("\tmiss eviction\n");miss += 1; eviction += 1; break;}}
        else {
            switch (res) {
                case 1: hit += 1; break;
                case 2: miss += 1; break;
                case 3: miss += 1; eviction += 1; break;}}
    }

    printSummary(hit, miss, eviction);
    return 0;
}
