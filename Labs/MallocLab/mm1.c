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
 * Given macros
 ********************************************************/
#define ALIGNMENT 8 // single word (4) or double word (8) alignment
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7) // rounds up to the nearest multiple of ALIGNMENT


/* macros from book */
#define WSIZE               4
#define DSIZE               8
#define uint                unsigned int
#define CHUNKSIZE           (1<<12) // 4K
#define MAX(x, y)           ((x)>(y)? (x) : (y))
#define PACK(size, alloc)   ((size) | (alloc))
#define GET(p)              (*(uint *)(p))
#define PUT(p, val)         (*(uint *)(p) = (val))
#define UNPACK_SIZE(p)      (GET(p) & (~0x7))
#define UNPACK_ALLOC(p)     (GET(p) & (0x1))
#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_BLK_SIZE(bp) - DSIZE)
#define NEXT_BLKP(bp)       ((char *)(bp) + GET_BLK_SIZE(bp))
#define PREV_BLKP(bp)       ((char *)(bp) - UNPACK_SIZE(HDRP(bp) - WSIZE))
#define SET_BOUNDARY(bp, size, alloc)    do {PUT(HDRP(bp), PACK(size, alloc)); PUT(FTRP(bp), PACK(size, alloc));} while (0)
#define LAST_FTR            (mem_heap_hi()+1-2*WSIZE)

/*********************************************************
 * self defined macros
 ********************************************************/
#define GET_BLK_SIZE(bp)    (UNPACK_SIZE(HDRP(bp)))
#define GET_BLK_ALLOC(bp)   (UNPACK_ALLOC(HDRP(bp)))

void * list_head;
static void * find_fit(size_t);
static void * coalesce(void *);
static void * extend_heap(size_t);
static void * place(void *, size_t);
void show_list();


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void* tmp;
    if ((tmp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;

    PUT(tmp, 0);
    SET_BOUNDARY(tmp+2*WSIZE, 8, 1);
    PUT(tmp+3*WSIZE, PACK(0    , 1));
    list_head = tmp+2*WSIZE;

    void * res_extrend_heap;
    if ((res_extrend_heap=extend_heap(CHUNKSIZE) == NULL))
        return -1;

    return 0;
}

static void * extend_heap(size_t real_bytes)
{
    void* bp;
    if ((bp = mem_sbrk(real_bytes)) == (void *)-1)
        return NULL;

    SET_BOUNDARY(bp, real_bytes, 0);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    bp = coalesce(bp);
    return bp;
}

static void * coalesce(void * bp)
{
    size_t prev_alloc_status = GET_BLK_ALLOC(PREV_BLKP(bp));  // 0-free, 1-alloced
    size_t prev_size         = GET_BLK_SIZE(PREV_BLKP(bp));
    size_t next_alloc_status = GET_BLK_ALLOC(NEXT_BLKP(bp));
    size_t next_size         = GET_BLK_SIZE(NEXT_BLKP(bp));
    size_t curr_size         = GET_BLK_SIZE(bp);

    if (prev_alloc_status && next_alloc_status){
        return bp;
    }

    // prev is free, next is alloced
    else if (!prev_alloc_status && next_alloc_status){
        size_t new_size = curr_size + prev_size;
        SET_BOUNDARY(PREV_BLKP(bp), new_size, 0);
        bp = PREV_BLKP(bp);
    }

    // prev is alloced, next is free
    else if (prev_alloc_status && !next_alloc_status){
        size_t new_size = curr_size + next_size;
        SET_BOUNDARY(bp, new_size, 0);
    }

    // prev is free, next is free
    else{
        size_t new_size = curr_size + next_size + prev_size;
        SET_BOUNDARY(PREV_BLKP(bp), new_size, 0);
        bp = PREV_BLKP(bp);
    }
    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

    if (size == 0) return NULL;

    int aligned_size = ALIGN(size) + DSIZE;  // full block size
    int extend_size = aligned_size;

    // search
    void * bp;
    if ((bp = find_fit(aligned_size)) != NULL){
        return place(bp, aligned_size);
    }

    if ((bp = extend_heap(extend_size)) == NULL) return NULL;
    return place(bp, aligned_size);
}

static void * find_fit(size_t size)
{
    for (void * tmp=list_head; GET_BLK_SIZE(tmp); tmp=NEXT_BLKP(tmp)) 
    {
        if ((!GET_BLK_ALLOC((tmp))) && (GET_BLK_SIZE(tmp) >= size))
            return tmp;
    }
    
    return NULL;
}

static void *place(void * bp, size_t size)
{
    size_t curr_size = GET_BLK_SIZE(bp);

    // no need to split
    if (curr_size - size < 2*DSIZE) {
        SET_BOUNDARY(bp, curr_size, 1);
        return bp; 
    }
    

    // split the block
    SET_BOUNDARY(bp, curr_size-size, 0);
    SET_BOUNDARY(NEXT_BLKP(bp), size, 1);
    coalesce(bp);
    return NEXT_BLKP(bp);

    // SET_BOUNDARY(bp, size, 1);
    // SET_BOUNDARY(NEXT_BLKP(bp), curr_size-size, 0);
    // coalesce(NEXT_BLKP(bp));
    // return bp;
}

void mm_free(void *bp)
{
    SET_BOUNDARY(bp, GET_BLK_SIZE(bp), 0);
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t req_size)
{
    if (bp == NULL) { return mm_malloc(req_size); }
    if (req_size == 0) { mm_free(bp); }

    size_t req_size_aligned = ALIGN(req_size) + DSIZE;
    size_t curr_size = GET_BLK_SIZE(bp);


    if (req_size_aligned <= curr_size) return bp;


    if (FTRP(bp) == LAST_FTR){
        extend_heap(req_size_aligned - curr_size);
        SET_BOUNDARY(bp, req_size_aligned, 1);
        return bp;
    }


    // malloc, memcpy, free
    void *oldbp = bp;
    void *newbp;
    size_t copySize;

    int overall_size = curr_size + GET_BLK_SIZE(NEXT_BLKP(bp));
    int next_blk_size = overall_size - req_size_aligned;

    if (GET_BLK_ALLOC(NEXT_BLKP(bp)) || next_blk_size<0)
    {
        copySize = GET_BLK_SIZE(oldbp) - DSIZE;
        newbp = mm_malloc(req_size);
        memcpy(newbp, oldbp, copySize);
        mm_free(oldbp);
        return newbp;
    }

    
    if (next_blk_size >= 2*DSIZE)
    {
        SET_BOUNDARY(bp, req_size_aligned, 1);
        SET_BOUNDARY(NEXT_BLKP(bp), next_blk_size, 0);
        coalesce(NEXT_BLKP(bp));
        return bp;
    }
    else
    {
        SET_BOUNDARY(bp, overall_size, 1);
        return bp;
    }
}


void show_list()
{
    void * tmp = list_head;
    int i=0;
        printf("----------------- HEAP LIST -----------------\n");
        printf("    DEBUG [show list]: %-10s \t ALLOC \t SIZE   \t GET_HDR\n", "BP");
    for (; GET(HDRP(tmp))!=PACK(0, 1) && (i<10); tmp=NEXT_BLKP(tmp)) {
        printf("    DEBUG [show list]: %-p \t %-5d \t %-6d \t %d\n", tmp, (int)(GET_ALLOC(HDRP(tmp))), (unsigned int)(GET_SIZE(HDRP(tmp))), GET(HDRP(tmp)));
        i++;
    }
        printf("    DEBUG [show list]: %-p \t %-5d \t %-6d \t %d\n", tmp, (int)(GET_ALLOC(HDRP(tmp))), (unsigned int)(GET_SIZE(HDRP(tmp))), GET(HDRP(tmp)));
        printf("---------------------------------------------\n");
}
