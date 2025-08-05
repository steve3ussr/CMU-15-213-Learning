/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Lier Team",
    /* First member's full name */
    "Steve Zhao",
    /* First member's email address */
    "steve.zhao@tongji.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*********************************************************
 * Macros
 ********************************************************/
// ALIGN
#define ALIGNMENT 4 // single word (4) or double word (8) alignment
#define ALIGN_TO_FULLBLK(size)      (((size+3) & (~0x7)) + DSIZE)

// DATA SIZE
#define uint                unsigned int
#define WSIZE               4
#define DSIZE               8
#define CHUNKSIZE           (1<<12) // 4K
#define MAX(x, y)           ((x)>(y) ? (x):(y))
#define MIN(x, y)           ((x)<(y) ? (x):(y))

// ACCESS TO BLOCK
#define GET(p)              ((void *)(*(uint *)(p)))
#define PUT(p, val)         (*(uint *)(p) = (uint)(val))
#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define NEXT_BLKP(bp)       GET(bp)
#define GET_BLK_SIZE(bp)    (size_t)(GET(HDRP(bp)))

#define FDP(bp)             (bp)
#define FD(bp)              (GET(FDP(bp)))
#define LAST_FTR            (mem_heap_hi()+1-2*WSIZE)



/*********************************************************
 * Global vars
 ********************************************************/
void   *list_8,  *list_16,   *list_32,   *list_64,  *list_128;
void *list_256, *list_512, *list_1024, *list_2048, *list_4096;
void *list_jumbo;
#define LIST_NUM                11
#define BLOCK_NUM_LIMIT         10
#define INIT_LIST(list, size)   do {PUT(list, 0); PUT(HDRP(list), size);} while (0)

static void *list_map(size_t);
static void *find_prev_by_size(size_t);
static void *extend_place_jumbo(size_t);
static void *extend_heap(size_t);
static void *place_curr(void *);
static void *find_prev_by_addr(void *);
static void coalesce(void *);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
    void *tmp;
    if ((tmp = mem_sbrk((LIST_NUM+2)*DSIZE)) == (void *)-1)
        return -1;

    list_8        = tmp+DSIZE*0 ; INIT_LIST(list_8,     8);
    list_16       = tmp+DSIZE*1 ; INIT_LIST(list_16,    16);
    list_32       = tmp+DSIZE*2 ; INIT_LIST(list_32,    32);
    list_64       = tmp+DSIZE*3 ; INIT_LIST(list_64,    64);
    list_128      = tmp+DSIZE*4 ; INIT_LIST(list_128,   128);
    list_256      = tmp+DSIZE*5 ; INIT_LIST(list_256,   256);
    list_512      = tmp+DSIZE*6 ; INIT_LIST(list_512,   512);
    list_1024     = tmp+DSIZE*7 ; INIT_LIST(list_1024,  1024);
    list_2048     = tmp+DSIZE*8 ; INIT_LIST(list_2048,  2048);
    list_4096     = tmp+DSIZE*9; INIT_LIST(list_4096,  4096);
    list_jumbo    = tmp+DSIZE*10; INIT_LIST(list_jumbo, 0);
    PUT(tmp+DSIZE*11, 1);
    PUT(tmp+DSIZE*10+WSIZE, 1);

    #if defined(DEBUG) 
        printf("\nDEBUG [mm_init]: finished. \n");
        printf("DEBUG [mm_init][8\t]: %p\n", list_8);
        printf("DEBUG [mm_init][16\t]: %p\n", list_16);
        printf("DEBUG [mm_init][32\t]: %p\n", list_32);
        printf("DEBUG [mm_init][64\t]: %p\n", list_64);
        printf("DEBUG [mm_init][128\t]: %p\n", list_128);
        printf("DEBUG [mm_init][256\t]: %p\n", list_256);
        printf("DEBUG [mm_init][512\t]: %p\n", list_512);
        printf("DEBUG [mm_init][1024\t]: %p\n", list_1024);
        printf("DEBUG [mm_init][2048\t]: %p\n", list_2048);
        printf("DEBUG [mm_init][4096\t]: %p\n", list_4096);
        printf("DEBUG [mm_init][jumbo\t]: %p\n", list_jumbo);
    #endif
    return 0;
}

static void *extend_heap(size_t aligned_size) {
    // bp=sbrk, fill HDR FTR
    void * bp;
    if ((bp = mem_sbrk(aligned_size)) == (void *)-1)
        return NULL;
    PUT(HDRP(bp), aligned_size);
    return bp;
}

static void *list_map(size_t size){
    void *res;
    if (size <= 8)          res = list_8;
    else if (size <= 16)    res = list_16;
    else if (size <= 32)    res = list_32;
    else if (size <= 64)    res = list_64;
    else if (size <= 128)   res = list_128;
    else if (size <= 256)   res = list_256;
    else if (size <= 512)   res = list_512;
    else if (size <= 1024)  res = list_1024;
    else if (size <= 2048)  res = list_2048;
    else if (size <= 4096)  res = list_4096;
    else                    res = list_jumbo;
    return res;
}

void *mm_malloc(size_t size){
    // align 4->8, 12->16
    size_t size_blk = ALIGN_TO_FULLBLK(size);
    void *head = list_map(size_blk);

    // normal list
    if (head != list_jumbo) {
        if (!FD(head)) {
            return extend_heap(GET_BLK_SIZE(head));
        }
        return place_curr(head);
    }
    
    // jumbo list
    else {
        void *prev = find_prev_by_size(size_blk);
        if (!prev) {
            return extend_heap(size_blk);
        }
        return place_curr(prev);}
}

static void *find_prev_by_size(size_t size){
    void *prev=list_jumbo;
    for (void *curr=FD(prev); curr; prev=curr, curr=FD(curr))
    {
        if (GET_BLK_SIZE(curr) >= size) return prev;
    }
    
    return NULL;
}

static void *place_curr(void *prev){
    void *bp = FD(prev);
    PUT(FDP(prev), FD(bp));
    return bp;
}

void mm_free(void *bp){
    size_t size = GET_BLK_SIZE(bp);
    void *head = list_map(size);

    // normal list
    if (head != list_jumbo) {
        void *next = FD(head);
        PUT(FDP(head), bp);
        PUT(FDP(bp), next);
        return;
    }

    //jumbo list
    #if defined(DEBUG) 
        printf("DEBUG [mm_free][jumbo]: freeing\n");
    #endif
    void *prev = find_prev_by_addr(bp);
    void *next = FD(prev);
    PUT(FDP(prev), bp);
    PUT(FDP(bp), next);

    coalesce(bp);
    if (prev != list_jumbo) coalesce(prev);
}

static void *find_prev_by_addr(void *bp){
    void *prev = list_jumbo;
    for (void *curr=FD(prev); curr; prev=curr, curr=FD(curr))
    {
        if (curr > bp) return prev;
    }
    return prev;
}

static void coalesce(void *bp){
    void *next = FD(bp);
    
    if (!next) return;

    size_t bp_size = GET_BLK_SIZE(bp);
    size_t next_size = GET_BLK_SIZE(next);

    if ( (bp+bp_size) != next) return;

    PUT(bp, FD(next));
    PUT(HDRP(bp), bp_size+next_size);
    // printf("triggered. \n");
}

void *mm_realloc(void *bp, size_t req_size){
    if (bp == NULL) { return mm_malloc(req_size); }
    if (req_size == 0) { mm_free(bp); }

    size_t bp_size = GET_BLK_SIZE(bp);
    size_t req_size_aligned = ALIGN_TO_FULLBLK(req_size);
    if (req_size_aligned <= bp_size) return bp;


    void *oldbp = bp;
    void *newbp;
    size_t copySize;

    copySize = bp_size - WSIZE;
    newbp = mm_malloc(req_size);
    memcpy(newbp, oldbp, copySize);
    mm_free(oldbp);
    return newbp;
}
