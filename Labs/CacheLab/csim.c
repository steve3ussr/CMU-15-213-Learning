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
    Line *prev, *next;

    // curr->next && curr->next->next != NULL
    // curr->next != set->dumb_tail
    while (curr->next && curr->next->next != NULL) {
        if (curr->tag == input_t && curr->v == 1) break;
        curr = curr->next;
    }  // curr->next = dumb_tail / a matched line

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
        set->len += 1;
        return 2;
    }
    else {
        // miss + eviction, pop head and append
        Line *head = set->dumb_head->next;

        set->dumb_head->next = head->next;
        head->next->prev = set->dumb_head;
        free(head);
        return 3;
    }
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

void PrintSet(Set* s){
    Line* curr = s->dumb_head->next;
    int cnt = 0;
    while(curr != s->dumb_tail){
        printf("Set Line %d: v=%d, tag=%d\n", cnt+1, curr->v, curr->tag);
        cnt += 1;
        curr = curr->next;}
}

void PrintCache(Set* c[], int length) {
    for (int i = 0; i < length; i++) {
        printf("--- Set [%d] Info ---\n", i);
        printf("\t Set len: %d, limit: %d\n", c[i]->len, c[i]->limit);
        PrintSet(c[i]);
    }
}


int main(int argc, char *argv[])
{

// Part 0: exit if argc length error
if ((argc != 9) && (argc != 10))
    return 1;


// Part 1: load args
int i = 1;
if (argc == 10){
    // TODO: handle verbose
    char *verbose = argv[i++];}

int res = strcmp(argv[i], "-s")==0 &&
          strcmp(argv[i+2], "-E")==0 &&
          strcmp(argv[i+4], "-b")==0 &&
          strcmp(argv[i+6], "-t")==0;
if (!res) printf("WARNING! ");

int s = strtol(argv[i+1], NULL, 10);
int E = strtol(argv[i+3], NULL, 10);
int b = strtol(argv[i+5], NULL, 10);
char *t = argv[i+7];


// Part 2: load file, get line nums -> ENTRY_LENGTH
char string[31];
FILE* file = fopen(t, "r");
if (file == NULL) {
    fprintf(stderr, "can't open %s: %s\n", t, strerror(errno));
    exit(1);
}
int ENTRY_LENGTH = 0;
while (fgets(string, 100, file)) {
    // printf("%s", string);
    ENTRY_LENGTH += 1;
}
fclose(file);


// Part 3: load file, initial entries
file = fopen(t, "r");
Entry* entries[ENTRY_LENGTH];
char method;


uint tmp=0, addr=0, size=0;
i = 0;
int tmp_;
while (fgets(string, 100, file)) {

    if (string[0] == 'I') {
        ENTRY_LENGTH -= 1;
        continue;
    }

    tmp_ = sscanf(string, " %c 7%8x,%d", &method, &addr, &size);
    if (tmp_ != 3) tmp = sscanf(string, " %c %x,%d", &method, &addr, &size);

    entries[i] = malloc(sizeof(Entry));
    memcpy(&(entries[i]->method), &method, sizeof(method));
    memcpy(&(entries[i]->addr), &addr, sizeof(addr));
    memcpy(&(entries[i]->size), &size, sizeof(size));
    i += 1;
}


// Part 4: initial cache
Set* cache[1<<s];
InitCache(cache, s, E);


// Part 5: exec for each entry
uint mask_b = (1<<b)-1;
uint mask_s = ((1<<(s+b))-1) ^ mask_b;
uint mask_t = ((1<<(64))-1) ^ mask_s ^ mask_b;


int hit=0, miss=0, eviction=0;
for(int j=0; j<ENTRY_LENGTH; j++){
    uint addr_t = (entries[j]->addr & mask_t) >> (s+b);
    uint addr_s = (entries[j]->addr & mask_s) >> b;
if (argc == 10){
    printf("%c %x,%d", entries[j]->method, entries[j]->addr, entries[j]->size);
}
    method = entries[j]->method;
    if (method == 'M') {
        res = TraverseList(cache[addr_s], addr_t, 0);
        if (argc == 10) {
            switch (res) {
            case 1: printf("\thit", res); hit += 1; break;
            case 2: printf("\tmiss", res); miss += 1; break;
            case 3: printf("\tmiss eviction", res);miss += 1; eviction += 1; break;}}
        else {
            switch (res) {
            case 1: hit += 1; break;
            case 2: miss += 1; break;
            case 3: miss += 1; eviction += 1; break;}}
    }

    res = TraverseList(cache[addr_s], addr_t, 0);
    if (argc == 10) {
        switch (res) {
            case 1: printf("\thit\n", res); hit += 1; break;
            case 2: printf("\tmiss\n", res); miss += 1; break;
            case 3: printf("\tmiss eviction\n", res);miss += 1; eviction += 1; break;}}
    else {
        switch (res) {
            case 1: hit += 1; break;
            case 2: miss += 1; break;
            case 3: miss += 1; eviction += 1; break;}}
}
printSummary(hit, miss, eviction);

    return 0;
}
