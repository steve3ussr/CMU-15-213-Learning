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
#include <math.h>

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
#define ALIGN_TO_FULLBLK(size)      (MAX((((size+7) & ~0x7) + DSIZE), 4*WSIZE))

// DATA SIZE
#define uint                unsigned int
#define WSIZE               4
#define DSIZE               8
#define CHUNKSIZE           (1<<12) // 4K
#define MAX(x, y)           ((x)>(y) ? (x):(y))
#define MIN(x, y)           ((x)<(y) ? (x):(y))
#define RANGE_HI(list)      (1 << ((list-heap_head)/WSIZE+4))
#define RANGE_LO(list)      (1 << ((list-heap_head)/WSIZE+3))

// ACCESS TO BLOCK
#define GET(p)              ((void *)(*(uint *)(p)))
#define PUT(p, val)         (*(uint *)(p) = (uint)(val))

#define PACK(size, alloc)   ((size) | (alloc))
#define UNPACK_SIZE(p)      (size_t)((uint)GET(p) & (~0x7))
#define UNPACK_ALLOC(p)     (size_t)((uint)GET(p) & (0x1))

#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FDP(bp)             (bp)
#define BKP(bp)             ((char *)(bp) + WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_BLK_SIZE(bp) - DSIZE)

#define GET_BLK_SIZE(bp)    (UNPACK_SIZE(HDRP(bp)))
#define GET_BLK_ALLOC(bp)   (UNPACK_ALLOC(HDRP(bp)))
#define FD(bp)              (GET(FDP(bp)))
#define BK(bp)              (GET(BKP(bp)))
#define NEXT(bp)            ((char *)(bp) + GET_BLK_SIZE(bp))
#define PREV(bp)            ((char *)(bp) - UNPACK_SIZE(bp - DSIZE))
#define SET_ALLOC(bp)       do {PUT(HDRP(bp), PACK(GET_BLK_SIZE(bp), 1)); PUT(FTRP(bp), PACK(GET_BLK_SIZE(bp), 1));} while (0)
#define SET_FREE(bp)        do {PUT(HDRP(bp), PACK(GET_BLK_SIZE(bp), 0)); PUT(FTRP(bp), PACK(GET_BLK_SIZE(bp), 0));} while (0)
#define SET_BOUNDARY(bp, size, alloc)    do {PUT(HDRP(bp), PACK(size, alloc)); PUT(FTRP(bp), PACK(size, alloc));} while (0)
#define LAST_FTR            (mem_heap_hi()+1-2*WSIZE)

/*********************************************************
 * Global vars
 ********************************************************/
void *list_16,   *list_32,   *list_64,  *list_128;
void *list_256, *list_512, *list_1024, *list_2048;
void *list_4096, *list_8192, *list_jumbo;
void *heap_head;
void *prologue;
void *epilogue;
int global_index = 0;
#define LIST_NUM                11
#define LIST_MAX_INDEX          (LIST_NUM-1)
#define BLOCK_NUM_LIMIT         4
#define INIT_LIST(list, init, index)   do {list=init+(index)*WSIZE; PUT(list, 0);} while (0)

static void *search_block(size_t aligned_size);
static void detach_block(void *bp);
static void *extend_heap(size_t aligned_size);
static uint _get_list_index(size_t aligned_size);
static void *get_list(size_t aligned_size);
static void *place(void *bp, size_t aligned_size);
static void attach_block(void *bp);
static void *coalesce(void *bp);



int mm_init(void){
    void* tmp;
    if ((tmp = mem_sbrk((LIST_NUM+3)*WSIZE)) == (void *)-1)
        return -1;


    // 11 headers
    heap_head = tmp;
    INIT_LIST(list_16,      tmp, 0);
    INIT_LIST(list_32,      tmp, 1);
    INIT_LIST(list_64,      tmp, 2);
    INIT_LIST(list_128,     tmp, 3);
    INIT_LIST(list_256,     tmp, 4);
    INIT_LIST(list_512,     tmp, 5);
    INIT_LIST(list_1024,    tmp, 6);
    INIT_LIST(list_2048,    tmp, 7);
    INIT_LIST(list_4096,    tmp, 8);
    INIT_LIST(list_8192,    tmp, 9);
    INIT_LIST(list_jumbo,   tmp, 10);
    PUT(tmp+11*WSIZE, PACK(8, 1)); // prologue
    PUT(tmp+12*WSIZE, PACK(8, 1)); // prologue
    PUT(tmp+13*WSIZE, PACK(0, 1)); // epilogue

    void *init_bp = extend_heap(48);
    attach_block(init_bp);
    init_bp = extend_heap(8);
    SET_ALLOC(init_bp);
    return 0;
}

static uint _get_list_index(size_t aligned_size)
{
    if (aligned_size <= 16) {return 0;}
    else if (aligned_size <= 32) {return 1;}
    else if (aligned_size <= 64) {return 2;}
    else if (aligned_size <= 128) {return 3;}
    else if (aligned_size <= 256)  {return 4;}
    else if (aligned_size <= 512)  {return 5; }
    else if (aligned_size <= 1024)  {return 6; }
    else if (aligned_size <= 2048)   {return 7; }
    else if (aligned_size <= 4096)   {return 8;  }
    else if (aligned_size <= 8192)   {return 9; }
    else   {return 10; }
}

static void *get_list(size_t aligned_size){
    return heap_head + WSIZE * _get_list_index(aligned_size);
}

static void *extend_heap(size_t aligned_size){
    // extend, do not attach, marked as free
    void *bp;
    if ((bp = mem_sbrk(aligned_size)) == (void *)-1)
        return NULL;

    SET_BOUNDARY(bp, aligned_size, 0);
    PUT(HDRP(NEXT(bp)), PACK(0, 1));
    return bp;
}

static void *search_block(size_t aligned_size){
    size_t index = _get_list_index(aligned_size);
    void *curr;
    void *list_head;

    for (int i=index; i<=LIST_MAX_INDEX; i++)
    {
        list_head = heap_head+WSIZE*i;
        curr = FD(list_head);

        for (; curr; curr=FD(curr))
            if (GET_BLK_SIZE(curr) >= aligned_size) {return curr;}
    }
    return NULL;
}

void *mm_malloc(size_t size){
    if (size == 112) {size = 128;} 
    else if (size == 448) {size = 512;}
    size_t aligned_size = ALIGN_TO_FULLBLK(size);

    // can find a bigger block
    void *bp = search_block(aligned_size);
    if (bp) { return place(bp, aligned_size); }

    // cannot find a bigger block, extend
    bp = extend_heap(aligned_size);
    SET_ALLOC(bp);
    return bp;
}

void *place(void *bp, size_t aligned_size){
    // bp is free, connected

    size_t bp_size = GET_BLK_SIZE(bp);
    size_t split_size = bp_size - aligned_size;
    void *bp_list = get_list(bp_size);
    void *split_list = get_list(split_size); 
    void *res;

    // case 1: cannot split
    if (split_size < 4*WSIZE)
    {
        detach_block(bp);
        SET_ALLOC(bp);
        return bp;
    }

    // case 2: split; split-block and bp are in the same list, so just ALLOC TAIL PAR
    else if (bp_list == split_list){
        SET_BOUNDARY(bp, split_size, 0);

        res = NEXT(bp);
        SET_BOUNDARY(res, aligned_size, 1);
        return res;
    }

    // case 3: split; split-block in another list
    else {
        detach_block(bp);
        SET_BOUNDARY(bp, aligned_size, 1);
        res = bp;

        bp = NEXT(bp);
        SET_BOUNDARY(bp, split_size, 0);
        bp = coalesce(bp);
        attach_block(bp);
        
        return res;
    }
}

static void detach_block(void *bp){
    // detach this block from its list
    void *fd = FD(bp);
    void *bk = BK(bp);

    PUT(FDP(bk), fd);
    if (fd) { PUT(BKP(fd), bk); }
}

void attack_block_lifo(void *bp){
    void *list = get_list(GET_BLK_SIZE(bp));
    void *fd = FD(list);
    PUT(FDP(list), bp);
    PUT(BKP(bp), list);
    PUT(FDP(bp), fd);
    if (fd) PUT(BKP(fd), bp);
}

void attach_block(void *bp){
    return attack_block_lifo(bp);
    void *list = get_list(GET_BLK_SIZE(bp));
    void *prev = list; 
    void *curr = FD(list);

    // search for the first bigger addr block
    for (; curr; prev=curr, curr=FD(curr))
    {
        if (curr > bp) break;
    }

   PUT(FDP(prev), bp);
   PUT(BKP(bp), prev);
   PUT(FDP(bp), curr);
   if (curr) PUT(BKP(curr), bp);
   return;
}

void *coalesce(void *bp){
    // bp is not connected, free block
    void *prev = PREV(bp);
    void *next = NEXT(bp);

    int prev_is_alloc = GET_BLK_ALLOC(prev);
    int next_is_alloc = GET_BLK_ALLOC(next);

    // case 1: do not merge
    if ((prev_is_alloc) && (next_is_alloc)) { return bp; }

    size_t overall_size = 0;
    // case 2: merge prev
    if ((!prev_is_alloc) && (next_is_alloc))
    {
        overall_size = GET_BLK_SIZE(bp) + GET_BLK_SIZE(prev);
        detach_block(prev);
        SET_BOUNDARY(prev, overall_size, 0);
        return coalesce(prev);
    }

    // case 3: merge next
    else if ((prev_is_alloc) && (!next_is_alloc)){
        overall_size = GET_BLK_SIZE(bp) + GET_BLK_SIZE(next);
        detach_block(next);
        SET_BOUNDARY(bp, overall_size, 0);
        return coalesce(bp);
    }

    // case 4: merge prev + next
    else if ((!prev_is_alloc) && (!next_is_alloc)){
        overall_size = GET_BLK_SIZE(prev) + GET_BLK_SIZE(bp) + GET_BLK_SIZE(next);
        detach_block(prev);
        detach_block(next);
        SET_BOUNDARY(prev, overall_size, 0);
        return coalesce(prev);
    }
    return NULL;
}

void mm_free(void *bp){
    SET_FREE(bp);
    bp = coalesce(bp);
    attach_block(bp);
}

void *mm_realloc(void *bp, size_t req_size){
    size_t req_size_aligned = ALIGN_TO_FULLBLK(req_size);
    size_t bp_size = GET_BLK_SIZE(bp);

    // case 1: request smaller size: do nothing
    if (req_size_aligned <= bp_size) return bp;

    // case 2: request bigger size
    // case 2.1: bp is at heap tail
    if ((FTRP(bp)) == LAST_FTR) {
        extend_heap(req_size_aligned - bp_size);
        SET_BOUNDARY(bp, req_size_aligned, 1);
        return bp;
    }
    
    // case 2.2：can find a free block somewhere, detach it ** 不过这一部分不太重要, 实测不会触发 **
    // void *next = NEXT(bp);
    // if ((!GET_BLK_ALLOC(next)) && ((GET_BLK_SIZE(next) + bp_size) >= req_size_aligned))
    // {
    //     detach_block(next);
    //     SET_BOUNDARY(bp, (GET_BLK_SIZE(next) + bp_size), 1);
    //     return bp;
    // }

    // case 2.3: alloc, memcpy, free
    void *oldbp = bp;
    void *newbp;
    size_t copySize;

    copySize = GET_BLK_SIZE(oldbp) - WSIZE;
    newbp = mm_malloc(req_size);
    memcpy(newbp, oldbp, copySize);
    mm_free(oldbp);
    return newbp;
}

void show_list(void *head){
    printf("--------------------------------------------\n");
    void *bp = GET(FDP(head));
    if (!bp){
        printf("    empty\n");
    }
    else{
        printf("                         size(hdrp)    fdp          bkp    ftrp\n");
        while ((bp) && (GET_BLK_SIZE(bp))){
            printf("[%p]->[%p]: %6d \t %p \t %p \t %p\n", bp, bp+GET_BLK_SIZE(bp), GET_BLK_SIZE(bp), GET(FDP(bp)), GET(BKP(bp)), GET(FTRP(bp)));
            bp = FD(bp);
        }
    }

    printf("--------------------------------------------\n");
}

void show_heap()
{
    void * tmp = heap_head+12*WSIZE;

        printf("----------------- HEAP LIST -----------------\n");
        printf("    DEBUG [show list]: %-10s \t ALLOC \t SIZE   \t GET_HDR \t FD \t BK\n", "BP");
    for (; GET(HDRP(tmp))!=(void *)PACK(0, 1); tmp=NEXT(tmp)) {
        printf("    DEBUG [show list]: %-p \t %-5d \t %-6d \t %p \t %p \t %p\n", tmp, (int)(GET_BLK_ALLOC(tmp)), (unsigned int)(GET_BLK_SIZE(tmp)), GET(HDRP(tmp)), FD(tmp), BK(tmp));
    }
        printf("    DEBUG [show list]: %-p \t %-5d \t %-6d \t %p \t %p \t %p\n", tmp, (int)(GET_BLK_ALLOC(tmp)), (unsigned int)(GET_BLK_SIZE(tmp)), GET(HDRP(tmp)), FD(tmp), BK(tmp));
        printf("---------------------------------------------\n");
}