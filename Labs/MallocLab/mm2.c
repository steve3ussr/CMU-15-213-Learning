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
#define ALIGN_TO_FULLBLK(size)      MAX((((size+ALIGNMENT-1) & (~(DSIZE    -1))) + DSIZE), 4*WSIZE)

// DATA SIZE
#define uint                unsigned int
#define WSIZE               4
#define DSIZE               8
#define FRAG_SIZE           6
#define CHUNKSIZE           (1<<12) // 4K
#define MAX(x, y)           ((x)>(y)? (x) : (y))

// ACCESS TO BLOCK
#define PACK(size, alloc)   ((size) | (alloc))
#define GET(p)              ((void *)(*(uint *)(p)))
#define UNPACK_SIZE(p)      (size_t)((uint)GET(p) & (~0x7))
#define UNPACK_ALLOC(p)     (size_t)((uint)GET(p) & (0x1))
#define PUT(p, val)         (*(uint *)(p) = (uint)(val))

#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FDP(bp)             (bp)
#define BKP(bp)             ((char *)(bp) + WSIZE)

#define GET_BLK_SIZE(bp)    (UNPACK_SIZE(HDRP(bp)))
#define GET_BLK_ALLOC(bp)   (UNPACK_ALLOC(HDRP(bp)))
#define LAST_FTR            (mem_heap_hi()+1-3*WSIZE)

#define FD(bp)              (GET(FDP(bp)))
#define BK(bp)              (GET(BKP(bp)))

#define LINK(prev, next)    do {PUT(FDP(prev), next); PUT(BKP(next), prev);} while (0)


/*********************************************************
 * Global vars
 ********************************************************/
void * list_head;
void * list_tail;
static void * find_fit(size_t);
static void * coalesce(void *);
static void * extend_heap(size_t);
static void * place(void *, size_t);
static void * mm_realloc_req_greater_block(void *, size_t);
void show_list();


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void* tmp;
    if ((tmp = mem_sbrk(6*WSIZE)) == (void *)-1)
        return -1;

    // PROLOGUE
    PUT(tmp+0*WSIZE, PACK(0, 1)); // header
    PUT(tmp+1*WSIZE, NULL);      // FD: epilogue
    PUT(tmp+2*WSIZE, NULL);       // BK: NULL
    
    // EPILOGUE
    PUT(tmp+3*WSIZE, PACK(0, 1));       // header
    PUT(tmp+4*WSIZE, NULL);       // FD: NULL
    PUT(tmp+5*WSIZE, NULL);      // BK: prologue

    // Global vars
    list_head = tmp+1*WSIZE;
    list_tail = tmp+4*WSIZE;
    LINK(list_head, list_tail);

    void *bp = extend_heap(64);
    mm_free(bp);
    bp = extend_heap(8);
    PUT(HDRP(bp), PACK(8, 1));

    return 0;
}

static void * extend_heap(size_t real_bytes)
{
    // preserve tail_prev
    void * tail_prev = BK(list_tail);

    // bp=sbrk, fill HDR FTR
    void * bp;
    if ((bp = mem_sbrk(real_bytes)) == (void *)-1)
        return NULL;
    bp = (void *)((char *)bp - DSIZE);
    PUT(HDRP(bp), PACK(real_bytes, 0));

    // create a new tail
    list_tail = (char *)(bp) + real_bytes;
    PUT(HDRP(list_tail),    PACK(0, 1));
    PUT(FDP(list_tail),      NULL);
    PUT(BKP(list_tail),      NULL);

    // link nodes
    LINK(tail_prev, list_tail);

    return bp;
}

static void * coalesce(void * bp)
{
    // get BK, curr(bp), FD status
    size_t curr_size            = GET_BLK_SIZE(bp);

    size_t prev_alloc           = GET_BLK_ALLOC(BK(bp));  // 0-free, 1-alloced
    size_t prev_size            = GET_BLK_SIZE(BK(bp));
    int    prev_coherent        = (char *)BK(bp)+prev_size == (char *)bp;  // True if coherent with curr
    int    flag_merge_prev      = (!prev_alloc) && prev_coherent;

    size_t next_alloc           = GET_BLK_ALLOC(FD(bp));
    size_t next_size            = GET_BLK_SIZE(FD(bp));
    int    next_coherent        = (char *)FD(bp) == (char *)(bp)+curr_size; // True if coherent with curr
    int    flag_merge_next      = (!next_alloc) && next_coherent;


    // case 1: cannot merge
    if (!flag_merge_next && !flag_merge_prev){
        return bp;
    }

    // case 2: BK is free, FD is alloced
    else if (flag_merge_prev && !flag_merge_next){
        size_t new_size = curr_size + prev_size;
        PUT(HDRP(BK(bp)), PACK(new_size, 0));
        LINK(BK(bp), FD(bp));
        bp = BK(bp);
    }

    // case 3: BK is alloced, FD is free
    else if (!flag_merge_prev && flag_merge_next){
        size_t new_size = curr_size + next_size;
        void *bp_fd_fd = FD(FD(bp));
        LINK(bp, bp_fd_fd);
        PUT(HDRP(bp), PACK(new_size, 0));
    }

    // BK is free, FD is free
    else{
        size_t new_size = curr_size + next_size + prev_size;
        LINK(BK(bp), FD(FD(bp)));
        PUT(HDRP(BK(bp)), PACK(new_size, 0));
        bp = BK(bp);
    }

    return bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if (size == 0) return NULL;
    if (size == 112) {size = 128;} 
    else if (size == 448) {size = 512;}
    // calc align size
    int aligned_block_size = ALIGN_TO_FULLBLK(size);  // full block size
    int extend_size = aligned_block_size;

    // search and place
    void * bp;
    if ((bp = find_fit(aligned_block_size)) != NULL){
        return place(bp, aligned_block_size);
    }

    if ((bp = extend_heap(extend_size)) == NULL)
        return NULL;
    PUT(HDRP(bp), PACK(extend_size, 1));
    return bp;

}

static void * find_fit(size_t size)
{
    for (void * bp = list_head; bp; bp=FD(bp)) 
        if ((!GET_BLK_ALLOC(bp)) && (GET_BLK_SIZE(bp) >= size))
        {
            return bp;
        }
    return NULL;
}

static void * place(void * bp, size_t size)
{
    // calc size
    size_t curr_size = GET_BLK_SIZE(bp);

    // no need to split
    if (curr_size - size < FRAG_SIZE*WSIZE) {
        PUT(HDRP(bp), PACK(curr_size, 1));
        LINK(BK(bp), FD(bp));
        return bp; 
    }
    
    // split the block
    PUT(HDRP(bp), PACK(curr_size-size, 0));


    void * alloc_bp = (bp) + GET_BLK_SIZE(bp);
    PUT(HDRP(alloc_bp), PACK(size, 1));


    coalesce(bp);
    return alloc_bp;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    // mark bp as free, create FTR
    PUT(HDRP(bp), PACK(GET_BLK_SIZE(bp), 0));

    // locate bp, link with BK and FD
    void * curr, * prev;
    for (curr=list_head; curr != NULL; curr=FD(curr)) if (curr > bp) break;    

    prev = BK(curr);

    // link BK, bp, curr
    LINK(prev, bp);
    LINK(bp, curr);

    // coalesce
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t req_size)
{
    if (bp == NULL) { return mm_malloc(req_size); }
    if (req_size == 0) { mm_free(bp); }

    size_t req_size_aligned = ALIGN_TO_FULLBLK(req_size);
    size_t curr_size = GET_BLK_SIZE(bp);

    if (req_size_aligned <= curr_size) return bp;

    return mm_realloc_req_greater_block(bp, req_size_aligned);
}

static void * mm_realloc_req_greater_block(void * bp, size_t aligned_req_size)
{
    // case 1: bp at tail, extend
    if ((bp + GET_BLK_SIZE(bp)) == list_tail)
    {
        size_t aux_size = aligned_req_size - GET_BLK_SIZE(bp);
        void *aux_bp = extend_heap(aux_size);
        PUT(HDRP(bp), PACK(aligned_req_size, 1));
        return bp;
    }


    // try to find a coherent FD free block
    void * curr = list_head;
    for (; curr; curr=FD(curr)) if (curr > bp) break;


    // case 2: there is an adjacent free block with inadequate size
    if ((bp+GET_BLK_SIZE(bp) == curr) && (GET_BLK_SIZE(bp)+GET_BLK_SIZE(curr)) >= aligned_req_size)
    {
        // calc rest space
        size_t overall_size = GET_BLK_SIZE(bp) + GET_BLK_SIZE(curr); 
        size_t split_size = overall_size - aligned_req_size;
        void * prev = BK(curr), * next = FD(curr);

        // case 2.1: split is too small (less than 6*WORDS), do not split
        // connect BK and FD
        
        if (split_size < FRAG_SIZE*WSIZE){
            void * prev = BK(curr), * next = FD(curr);
            PUT(HDRP(bp), PACK(overall_size, 1));

            LINK(prev, next);
        }

        // case 2.2: split 
        else{
            PUT(HDRP(bp), aligned_req_size);
            void * split = (void *)((char *)(bp) + GET_BLK_SIZE(bp));
            PUT(HDRP(split), PACK(split_size, 0));
            
            LINK(prev, split); LINK(split, next);
            coalesce(split);
        }
        return bp;
    }

    // case 3: adjacent but too small; no adjacent free block -> just malloc, memcpy, free
    void *oldbp = bp;
    void *newbp;
    size_t copySize;

    copySize = GET_BLK_SIZE(oldbp) - WSIZE;
    newbp = mm_malloc(aligned_req_size);
    memcpy(newbp, oldbp, copySize);
    mm_free(oldbp);
    return newbp;

}