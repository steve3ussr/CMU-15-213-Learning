#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
        curr->prev = set->dumb_tail->prev->next;
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
Entry* BuildEntry(char file_line[]){
    Entry *res;
    sscanf(file_line, "%s %d,%d", &(res->method), &(res->addr), &(res->size));
    //printf("Original: %s\n", file_line);
    printf("\t%s, %d, %d\n", &(res->method), res->addr, res->size);
    //printf("----------------------\n");
}

int main(int argc, char *argv[]){
    const int ENTRY_LENGTH = 7;
    int s=1, E=1, b=1;
    char entry_method[] = {'L', 'M', 'L', 'S', 'L', 'L', 'M'};
    uint entry_addr[] = {0x10, 0x20, 0x22, 0x18, 0x110, 0x210, 0x12};
    uint entry_size[] = {1, 1, 1, 1, 1, 1, 1};

    Entry* entries[ENTRY_LENGTH];
    for(int i=0; i<ENTRY_LENGTH; i++) {
        entries[i] = malloc(sizeof(Entry));
        entries[i]->method = entry_method[i];
        entries[i]->addr   = entry_addr[i];
        entries[i]->size   = entry_size[i];}

    Set* cache[1<<s];
    InitCache(cache, s, E);


    uint mask_b = (1<<b)-1;
    uint mask_s = ((1<<(s+b))-1) ^ mask_b;
    uint mask_t = 0xffffffff ^ mask_s ^ mask_b;
    int res;
    char method;
    int hit=0, miss=0, eviction=0;
    for(int i=0; i<ENTRY_LENGTH; i++){
        uint addr = entries[i]->addr;
        uint addr_t = (addr & mask_t) >> (s+b);
        uint addr_s = (addr & mask_s) >> b;

        method = entries[i]->method;
        if (method == 'M') {
            res = TraverseList(cache[addr_s], addr_t, 0);
            switch(res){
                case 1:
                    hit += 1; break;
                case 2:
                    miss += 1; break;
                case 3:
                    miss += 1; eviction += 1; break;
            }
        }
        res = TraverseList(cache[addr_s], addr_t, 0);
        switch(res){
            case 1:
                hit += 1; break;
            case 2:
                miss += 1; break;
            case 3:
                miss += 1; eviction += 1; break;
        }
    }
    printSummary(hit, miss, eviction);


    return 0;
}
