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
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* macros from book */
#define WSIZE               4
#define DSIZE               8
#define CHUNKSIZE           (1<<12) // 4K
#define MAX(x, y)           ((x)>(y)? (x) : (y))
#define PACK(size, alloc)   ((size) | (alloc))
#define GET(p)              (*(unsigned int *)(p))
#define PUT(p, val)         (*(unsigned int *)(p) = (val))
#define GET_SIZE(p)         (GET(p) & (~0x7))
#define GET_ALLOC(p)        (GET(p) & (0x1))
#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_BLK_SIZE(bp) - DSIZE)
#define NEXT_BLKP(bp)       ((char *)(bp) + GET_BLK_SIZE(bp))
#define PREV_BLKP(bp)       ((char *)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

/*********************************************************
 * self defined macros
 ********************************************************/
#define GET_PREV_ALLOC(p)   (GET(p) & (0x4))
#define GET_NEXT_ALLOC(p)   (GET(p) & (0x2))
#define GET_BLK_SIZE(bp)    (GET_SIZE(HDRP(bp)))
#define GET_BLK_ALLOC(bp)   (GET_ALLOC(HDRP(bp)))

void * heap_listp;
static void * find_fit(size_t);
static void * coalesce(void *);
static void * extend_heap(size_t);
static void place(void *, size_t);
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
    PUT(tmp+1*WSIZE, PACK(DSIZE, 1));
    PUT(tmp+2*WSIZE, PACK(DSIZE, 1));
    PUT(tmp+3*WSIZE, PACK(0    , 1));
    heap_listp = tmp+2*WSIZE;
    #ifdef DEBUG
        printf("\nDEBUG [mm_init]: heap_listp is %p\n", heap_listp);
    #endif

    void * res_extrend_heap;
    if ((res_extrend_heap=extend_heap((size_t)(CHUNKSIZE/WSIZE))) == NULL)
        return -1;

    #ifdef DEBUG
        printf("DEBUG [mm_init]: extend heap start from %p\n", res_extrend_heap);
        show_list();
    #endif

    return 0;
}

static void * extend_heap(size_t words)
{
    size_t real_words = (words%2) ? (words+1) : words;
    size_t real_bytes = real_words * WSIZE;

    void* bp;
    if ((bp = mem_sbrk(real_bytes)) == (void *)-1)
        return NULL;

    PUT(HDRP(bp), PACK(real_bytes, 0));
    PUT(FTRP(bp), PACK(real_bytes, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    
    #ifdef DEBUG
        printf("DEBUG [extend_heap-sbrk]: sbrk return %p, extent %d words (%d real bytes)\n", bp, (int)words, (int)real_bytes);
    #endif
    bp = coalesce(bp);
    #ifdef DEBUG
        printf("DEBUG [extend_heap-coalesce]: bp is %p, size %d\n", bp, GET_BLK_SIZE(bp));
    #endif
    return bp;
}

static void * coalesce(void * bp)
{
    size_t prev_alloc_status = GET_BLK_ALLOC(PREV_BLKP(bp));  // 0-free, 1-alloced
    size_t prev_size         = GET_BLK_SIZE(PREV_BLKP(bp));
    size_t next_alloc_status = GET_BLK_ALLOC(NEXT_BLKP(bp));
    size_t next_size         = GET_BLK_SIZE(NEXT_BLKP(bp));
    size_t curr_size         = GET_BLK_SIZE(bp);

    #ifdef DEBUG
        printf("DEBUG [coalesce-info-curr]: coalescing %p\n", bp);
        printf("DEBUG [coalesce-info-prev]: size %d (alloc %d)\n", prev_size, prev_alloc_status);
        printf("DEBUG [coalesce-info-next]: size %d (alloc %d)\n", next_size, next_alloc_status);
    #endif


    if (prev_alloc_status && next_alloc_status){
        #ifdef DEBUG
            printf("DEBUG [coalesce-switch]: no need to coalesce\n");
        #endif
        return bp;
    }

    // prev is free, next is alloced
    else if (!prev_alloc_status && next_alloc_status){
        #ifdef DEBUG
            printf("DEBUG [coalesce-switch]: prev is free\n");
        #endif
        size_t new_size = curr_size + prev_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(new_size, 0));
        PUT(FTRP(bp), PACK(new_size, 0));
        bp = PREV_BLKP(bp);
    }

    // prev is alloced, next is free
    else if (prev_alloc_status && !next_alloc_status){
        #ifdef DEBUG
            printf("DEBUG [coalesce-switch]: next is free\n");
        #endif
        size_t new_size = curr_size + next_size;
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size, 0));
        PUT(HDRP(bp), PACK(new_size, 0));
    }

    // prev is free, next is free
    else{
        #ifdef DEBUG
            printf("DEBUG [coalesce-switch]: both are free\n");
        #endif
        size_t new_size = curr_size + next_size + prev_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(new_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size, 0));
        bp = PREV_BLKP(bp);
    }
    #ifdef DEBUG
        printf("DEBUG [coalesce-res]: return bp is %p\n", bp);
    #endif
    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    // int newsize = ALIGN(size + SIZE_T_SIZE);
    // void *p = mem_sbrk(newsize);
    // if (p == (void *)-1)
	// return NULL;
    // else {
    //     *(size_t *)p = size;
    //     return (void *)((char *)p + SIZE_T_SIZE);
    // }
    if (size == 0)
        return NULL;

    int aligned_size = ALIGN(size) + DSIZE;  // full block size
    int extend_size = MAX(aligned_size, CHUNKSIZE);

    // search
    void * bp;
    if ((bp = find_fit(aligned_size)) != NULL){
        #ifdef DEBUG
            printf("DEBUG [mm_malloc-find_fit]: placing size %d at %p\n", size, bp);
        #endif
        place(bp, aligned_size);
        #ifdef DEBUG
            show_list();
        #endif
        return bp;
    }

    if ((bp = extend_heap(extend_size/WSIZE)) == NULL)
        return NULL;
    #ifdef DEBUG
        printf("DEBUG [mm_malloc-extend_heap]: placing size %d at %p\n", size, bp);
    #endif
    place(bp, aligned_size);
    #ifdef DEBUG
        show_list();
    #endif
    return bp;
}

static void * find_fit(size_t size)
{
    for (void * tmp=heap_listp; GET(HDRP(tmp))!=PACK(0, 1); tmp=NEXT_BLKP(tmp)) {
        if ((!GET_BLK_ALLOC((tmp))) && (GET_BLK_SIZE(tmp) >= size))
        {
            #ifdef DEBUG
                printf("DEBUG [find_fit]: find fit finished. First-Fit bp is %p\n", tmp);
            #endif
            return tmp;
        }
    }
    
    // tmp now points to epilogue block
    return NULL;
}


static void place(void * bp, size_t size)
{
    size_t curr_size = GET_BLK_SIZE(bp);
    #ifdef DEBUG
        printf("DEBUG [place]: overall size is %d\n", curr_size);
    #endif
    // no need to split
    if (curr_size - size < 2*DSIZE) {
        #ifdef DEBUG
            printf("DEBUG [place-not-split]: placed\n");
        #endif
        PUT(HDRP(bp), PACK(curr_size, 1));
        PUT(FTRP(bp), PACK(curr_size, 1));
        return; 
    }
    

    // split the block
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    #ifdef DEBUG
        printf("DEBUG [place-split-curr]: place at %p, size is %d\n", bp, GET_BLK_SIZE(bp));
    #endif


    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(curr_size-size, 0));
    PUT(FTRP(bp), PACK(curr_size-size, 0));
    #ifdef DEBUG
        printf("DEBUG [place-split-next]: next free bp is %p, size is %d\n", bp, GET_BLK_SIZE(bp));
    #endif
    coalesce(bp);
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    #ifdef DEBUG
        printf("DEBUG [mm_free]: freeing %p\n", bp);
    #endif
    PUT(HDRP(bp), PACK(GET_BLK_SIZE(bp), 0));
    PUT(FTRP(bp), PACK(GET_BLK_SIZE(bp), 0));
    coalesce(bp);

    #ifdef DEBUG
        show_list();
    #endif
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
    #ifdef DEBUG
        printf("DEBUG [mm_realloc]: original size %d, aligned to %d, current block size is %d\n", 
               req_size, 
               req_size_aligned, 
               curr_size);
    #endif


    void *oldbp = bp;
    void *newbp;
    size_t copySize;


    // malloc, memcpy, free
    if (req_size_aligned > curr_size)
    {   
        int overall_size = curr_size + GET_BLK_SIZE(NEXT_BLKP(bp));
        int next_blk_size = overall_size - req_size_aligned;

        if (GET_BLK_ALLOC(NEXT_BLKP(bp)) || next_blk_size<0)
        {
            // copySize = *(size_t *)((char *)oldbp - SIZE_T_SIZE);
            copySize = GET_SIZE(HDRP(oldbp)) - DSIZE;
            newbp = mm_malloc(req_size);
            memcpy(newbp, oldbp, copySize);
            mm_free(oldbp);
            #ifdef DEBUG
                printf("DEBUG [mm_realloc-malloc-free]: copy size %d from %p to %p\n", copySize, oldbp, newbp);
                show_list();
            #endif
            return newbp;
        }

        
        
        if (next_blk_size >= 2*DSIZE)
        {
            PUT(HDRP(bp), PACK(req_size_aligned, 1));
            PUT(FTRP(bp), PACK(req_size_aligned, 1));
            
            void * next_bp  = NEXT_BLKP(bp);
            PUT(HDRP(next_bp), PACK(next_blk_size, 0));
            PUT(FTRP(next_bp), PACK(next_blk_size, 0));
            coalesce(next_bp);
            #ifdef DEBUG
                printf("DEBUG [mm_realloc-merge-split]: split a block at %p\n", next_bp);
                show_list();
            #endif
            return bp;
        }
        else
        {
            PUT(HDRP(bp), PACK(overall_size, 1));
            PUT(FTRP(bp), PACK(overall_size, 1));
            #ifdef DEBUG
                printf("DEBUG [mm_realloc-merge]: merge the next block\n");
                show_list();
            #endif
            return bp;
        }
        
    }

    // split to another block, coalesce
    else if (curr_size - req_size_aligned >= 2*DSIZE)
    {
        PUT(HDRP(bp), PACK(req_size_aligned, 1));
        PUT(FTRP(bp), PACK(req_size_aligned, 1));

        void * next_bp  = NEXT_BLKP(bp);
        PUT(HDRP(next_bp), PACK(curr_size-req_size_aligned, 0));
        PUT(FTRP(next_bp), PACK(curr_size-req_size_aligned, 0));
        
        coalesce(next_bp);
        #ifdef DEBUG
            printf("DEBUG [mm_realloc-split]: split a block size %d\n", curr_size-req_size_aligned);
            show_list();
        #endif
        return bp;
    }


    // do nothing
    else 
    {
        #ifdef DEBUG
            printf("DEBUG [mm_realloc-do-nothing]: \n");
            show_list();
        #endif
        return bp; 
    }

}

void show_list()
{
    void * tmp = heap_listp;
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
