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
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8


/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* self defined macros */
#define WSIZE               4
#define DSIZE               8
#define CHUNKSIZE           (1<<12)  // 4K
#define MAX(x, y)           ((x)>(y)? (x) : (y))
#define PACK(size, alloc)   ((size) | (alloc))
#define GET(p)              (*(unsigned int *)(p))
#define PUT(p, val)         (*(unsigned int *)(p) = (val))
#define GET_SIZE(p)         (GET(p) & (~0x7))
#define GET_ALLOC(p)        (GET(p) & (0x1))
#define GET_PREV_ALLOC(p)   (GET(p) & (0x2))
#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_SIZE(HDRP(bp)) - WSIZE)
#define NEXT_BLKP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)       ((char *)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

/* self defined vars */
void * heap_listp;  // the init bp

static void * find_fit(size_t);
static void * coalesce(void *);
static void * extend_heap(size_t);
static void place(void *, size_t);


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
        printf("\nDEBUG [mm_init]: first bp is %p\n", heap_listp);
    #endif

    void * res_extrend_heap;
    if ((res_extrend_heap=extend_heap((size_t)(CHUNKSIZE/WSIZE))) == NULL)
        return -1;

    #ifdef DEBUG
        printf("DEBUG [mm_init]: extend heap is %p\n", res_extrend_heap);
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
        printf("DEBUG [extend_heap]: mem_sbrk return %p, extent words %d (real bytes %d); coalesce now\n", bp, (int)words, (int)real_bytes);
    #endif
    bp = coalesce(bp);
    #ifdef DEBUG
        printf("DEBUG [extend_heap]: coalesced bp is %p\n", bp);
    #endif
    return bp;
}

static void * coalesce(void * bp)
{
    size_t prev_alloc_status = GET_ALLOC(HDRP(PREV_BLKP(bp)));  // 0-free, 1-alloced
    size_t prev_size         = GET_SIZE(HDRP(PREV_BLKP(bp)));
    size_t next_alloc_status = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t next_size         = GET_SIZE(HDRP(NEXT_BLKP(bp)));
    size_t curr_size         = GET_SIZE(HDRP(bp));

    if (prev_alloc_status && next_alloc_status){
        #ifdef DEBUG
            printf("DEBUG [coalesce]: no need to coalesce\n");
        #endif
        return bp;
    }

    // prev is free, next is alloced
    else if (!prev_alloc_status && next_alloc_status){
        #ifdef DEBUG
            printf("DEBUG [coalesce]: prev is free\n");
        #endif
        size_t new_size = curr_size + prev_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(new_size, 0));
        PUT(FTRP(bp), PACK(new_size, 0));
        bp = PREV_BLKP(bp);
    }

    // prev is alloced, next is free
    else if (prev_alloc_status && !next_alloc_status){
        #ifdef DEBUG
            printf("DEBUG [coalesce]: next is free\n");
        #endif
        size_t new_size = curr_size + next_size;
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size, 0));
        PUT(HDRP(bp), PACK(new_size, 0));
    }

    // prev is free, next is free
    else{
        #ifdef DEBUG
            printf("DEBUG [coalesce]: both are free\n");
        #endif
        size_t new_size = curr_size + next_size + prev_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(new_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size, 0));
    }
    #ifdef DEBUG
        printf("DEBUG [coalesce]: return bp is %p\n", bp);
    #endif
    return bp;
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

    size_t aligned_size = ALIGN(size) + DSIZE;  // full block size
    size_t extend_size = MAX(aligned_size, CHUNKSIZE);

    // search
    void * bp;
    if ((bp = find_fit(aligned_size)) != NULL){
        #ifdef DEBUG
            printf("DEBUG [mm_malloc]: find a fit, placing size %d at %p\n", (int)(size), bp);
        #endif
        place(bp, aligned_size);
        return bp;
    }

    if ((bp = extend_heap(extend_size/WSIZE)) == NULL)
        return NULL;
    #ifdef DEBUG
        printf("DEBUG [mm_malloc]: find not fit, extend heap; placing size %d at %p\n", (int)(size), bp);
    #endif
    place(bp, aligned_size);
    return bp;

}

static void * find_fit(size_t size)
{
    #ifdef DEBUG
        printf("DEBUG [find_fit]: start to find\n\n");
        printf("    DEBUG [find_fit]: BP               ALLOC SIZE  \n");
    #endif
    void * tmp=heap_listp;
    tmp = NEXT_BLKP(tmp);
    tmp = NEXT_BLKP(tmp);
    #ifdef DEBUG
        printf("    DEBUG [find_fit]: BP %p    %d     %d  \n", tmp, (int)(GET_ALLOC(HDRP(tmp))), (unsigned int)(GET_SIZE(HDRP(tmp))));
        printf("    DEBUG [find_fit]: %d, %d, %d\n", GET(tmp), PACK(0, 1), GET(tmp)!=PACK(0, 1));
    #endif
    return NULL;



    for (tmp=heap_listp; GET(tmp)!=PACK(0, 1); tmp=NEXT_BLKP(tmp)) {
        #ifdef DEBUG
            printf("    DEBUG [find_fit]: BP %p    %d     %d  \n", tmp, (int)(GET_ALLOC(HDRP(tmp))), (unsigned int)(GET_SIZE(HDRP(tmp))));
        #endif

        if (!GET_ALLOC(HDRP(tmp)))
        {
            #ifdef DEBUG
                printf("    DEBUG [find_fit]:                            this block is free\n");
            #endif
        }
        else 
        {
            #ifdef DEBUG
                printf("    DEBUG [find_fit]:                            this block is allocated\n");
            #endif
        }


        if (GET_SIZE(HDRP(tmp)) >= size)
        {
            #ifdef DEBUG
                printf("    DEBUG [find_fit]: this block is big enough\n");
            #endif
        }

        if ((!GET_ALLOC(HDRP(tmp))) && (GET_SIZE(HDRP(tmp)) >= size))
            #ifdef DEBUG
                printf("DEBUG [find_fit]: find fit finished. First-Fit bp is %p\n", tmp);
            #endif
            return tmp;
    }
    
    // tmp now points to epilogue block
    return NULL;
}


static void place(void * bp, size_t size)
{
    size_t curr_size = GET_SIZE(HDRP(bp));

    // no need to split
    if (curr_size - size < 2*DSIZE) {
        #ifdef DEBUG
            printf("DEBUG [place]: no need to split\n");
        #endif
        PUT(HDRP(bp), PACK(curr_size, 1));
        PUT(FTRP(bp), PACK(curr_size, 1));
        return; 
    }
    

    // split the block
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    #ifdef DEBUG
        printf("DEBUG [place]: split; curr bp is %p\n", bp);
    #endif

    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(curr_size-size, 0));
    PUT(FTRP(bp), PACK(curr_size-size, 0));
    #ifdef DEBUG
        printf("DEBUG [place]: split; empty bp is %p, size is %d\n", bp, (int)(curr_size-size));
    #endif
    coalesce(bp);
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    #ifdef DEBUG
        printf("DEBUG [mm_free]: freeing %p\n", ptr);
    #endif
    PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
    PUT(FTRP(ptr), PACK(GET_SIZE(FTRP(ptr)), 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL){
        return mm_malloc(size);
    }
    if (size == 0){
        mm_free(ptr);
    }

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














