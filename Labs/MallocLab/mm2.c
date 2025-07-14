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
#define ALIGN_TO_ALIGNMENT(size)    (((size+ALIGNMENT-1) & (~(ALIGNMENT-1)))) // rounds up to the nearest multiple of ALIGNMENT
#define ALIGN_TO_PAYLOAD(size)      (((size+ALIGNMENT-1) & (~(DSIZE    -1))) + WSIZE)
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
#define GET(p)              (*(uint *)(p))
#define PUT(p, val)         (*(uint *)(p) = (uint)(val))
#define GET_SIZE(p)         (GET(p) & (~0x7))
#define GET_ALLOC(p)        (GET(p) & (0x1))
#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_BLK_SIZE(bp) - DSIZE)
#define NEXT_BLKP(bp)       ((void *)(*(uint *)(bp)))
#define PREV_BLKP(bp)       ((void *)(*(uint *)((char *)bp + WSIZE)))
#define FD(bp)              (bp)
#define BK(bp)              ((char *)(bp) + WSIZE)
#define GET_PREV_ALLOC(p)   (GET(p) & (0x4))
#define GET_NEXT_ALLOC(p)   (GET(p) & (0x2))
#define GET_BLK_SIZE(bp)    (GET_SIZE(HDRP(bp)))
#define GET_BLK_ALLOC(bp)   (GET_ALLOC(HDRP(bp)))

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
static void * mm_realloc_req_smaller_block(void *, size_t);
void show_list();


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void* tmp;
    if ((tmp = mem_sbrk(8*WSIZE)) == (void *)-1)
        return -1;

    // empty
    PUT(tmp, 0);

    // PROLOGUE
    PUT(tmp+1*WSIZE, PACK(4*WSIZE, 1)); // header
    PUT(tmp+2*WSIZE, tmp+6*WSIZE);      // FD: epilogue
    PUT(tmp+3*WSIZE, PACK(0, 0));       // BK: NULL
    PUT(tmp+4*WSIZE, PACK(4*WSIZE, 1)); // footer
    
    // EPILOGUE
    PUT(tmp+5*WSIZE, PACK(0, 1));       // header
    PUT(tmp+6*WSIZE, PACK(0, 0));       // FD: NULL
    PUT(tmp+7*WSIZE, tmp+2*WSIZE);      // BK: prologue

    // Global vars
        list_head = tmp+2*WSIZE;
        list_tail = tmp+6*WSIZE;
    
    #if defined(DEBUG_MM_INIT) || defined(DEBUG) 
        printf("\nDEBUG [mm_init]: prologue, epilogue set\n");
        show_list();
    #endif

    // Extend heap by chunksize (manage FD/BK inside extend_heap)
    if ((extend_heap((size_t)(CHUNKSIZE/WSIZE))) == NULL)
        return -1;

    #if defined(DEBUG_MM_INIT) || defined(DEBUG) 
    printf("\nDEBUG [mm_init]: head is %p, tail is %p\n", list_head, list_tail);
        show_list();
    #endif

    return 0;
}

static void * extend_heap(size_t words)
{
    size_t real_words = (words%2) ? (words+1) : words;
    size_t real_bytes = real_words * WSIZE;

    // preserve tail_prev
    void * tail_prev = PREV_BLKP(list_tail);

    // bp=sbrk, fill HDR FTR
    void * bp;
    if ((bp = mem_sbrk(real_bytes)) == (void *)-1)
        return NULL;

    bp = (void *)((char *)bp - DSIZE);




    PUT(HDRP(bp), PACK(real_bytes, 0));
    PUT(FTRP(bp), PACK(real_bytes, 0));

    #if defined(DEBUG_EXTEND) || defined(DEBUG) 
        printf("DEBUG [extend_heap][sbrk]: sbrk return %p, extent %d words (%d real bytes)\n", bp, (int)words, (int)real_bytes);
    #endif

    // create a new tail
    list_tail = (char *)(bp) + real_bytes;
    PUT(HDRP(list_tail),    PACK(0, 1));
    PUT(FD(list_tail),      PACK(0, 0));
    PUT(BK(list_tail),      bp);
    #if defined(DEBUG_EXTEND) || defined(DEBUG) 
        printf("DEBUG [extend_heap][epilogue]: new epilogue at %p\n", list_tail);
    #endif


    // mod tail_prev
    PUT(FD(tail_prev), bp);

    // mod bp
    PUT(FD(bp), list_tail);
    PUT(BK(bp), tail_prev);

    bp = coalesce(bp);
    #if defined(DEBUG_EXTEND) || defined(DEBUG) 
        printf("DEBUG [extend_heap][coalesce]: final bp is %p, size %d\n", bp, GET_BLK_SIZE(bp));
    #endif
    return bp;
}

static void * coalesce(void * bp)
{
    
    // get prev, curr(bp), next status
    size_t curr_size            = GET_BLK_SIZE(bp);

    size_t prev_alloc           = GET_BLK_ALLOC(PREV_BLKP(bp));  // 0-free, 1-alloced
    size_t prev_size            = GET_BLK_SIZE(PREV_BLKP(bp));
    int    prev_coherent        = (char *)PREV_BLKP(bp)+prev_size == (char *)bp;  // True if coherent with curr
    int    flag_merge_prev      = (!prev_alloc) && prev_coherent;

    size_t next_alloc           = GET_BLK_ALLOC(NEXT_BLKP(bp));
    size_t next_size            = GET_BLK_SIZE(NEXT_BLKP(bp));
    int    next_coherent        = (char *)NEXT_BLKP(bp) == (char *)(bp)+curr_size; // True if coherent with curr
    int    flag_merge_next      = (!next_alloc) && next_coherent;
    #if defined(DEBUG_COALESCE) || defined(DEBUG) 
        printf("DEBUG [coalesce][info][curr]: coalescing %p\n", bp);
        printf("DEBUG [coalesce][info][prev]: size %d, alloc %d, coherent%d\n", prev_size, prev_alloc, prev_coherent);
        printf("DEBUG [coalesce][info][next]: size %d, alloc %d, coherent%d\n", next_size, next_alloc, next_coherent);
    #endif

    // case 1: cannot merge
    if (!flag_merge_next && !flag_merge_prev){
        #if defined(DEBUG_COALESCE) || defined(DEBUG) 
            printf("DEBUG [coalesce][case]: no need to coalesce\n");
        #endif
        return bp;
    }

    // case 2: prev is free, next is alloced
    else if (flag_merge_prev && !flag_merge_next){
        #if defined(DEBUG) || defined(DEBUG) 
            printf("DEBUG [coalesce][case]: prev is free\n");
        #endif
        size_t new_size = curr_size + prev_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(new_size, 0));
        PUT(FTRP(bp), PACK(new_size, 0));
        PUT(FD(PREV_BLKP(bp)), NEXT_BLKP(bp));
        PUT(BK(NEXT_BLKP(bp)), PREV_BLKP(bp));
        bp = PREV_BLKP(bp);
    }

    // case 3: prev is alloced, next is free
    else if (!flag_merge_prev && flag_merge_next){
        #if defined(DEBUG_COALESCE) || defined(DEBUG) 
            printf("DEBUG [coalesce][case]: next is free\n");
        #endif
        size_t new_size = curr_size + next_size;

        PUT(BK(NEXT_BLKP(NEXT_BLKP(bp))), bp);
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size, 0));
        PUT(FD(bp), NEXT_BLKP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(new_size, 0));
    }

    // prev is free, next is free
    else{
        #if defined(DEBUG_COALESCE) || defined(DEBUG) 
            printf("DEBUG [coalesce][case]: both are free\n");
        #endif
        size_t new_size = curr_size + next_size + prev_size;

        PUT(BK(NEXT_BLKP(NEXT_BLKP(bp))), PREV_BLKP(bp));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(new_size, 0));
        PUT(FD(PREV_BLKP(bp)), NEXT_BLKP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
    }

    #if defined(DEBUG_COALESCE) || defined(DEBUG) 
        printf("DEBUG [coalesce][res]: return bp is %p\n", bp);
    #endif
    return bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

    if (size == 0)
        return NULL;

    // calc align size
    int aligned_block_size = ALIGN_TO_FULLBLK(size);  // full block size
    int extend_size = MAX(aligned_block_size, CHUNKSIZE);

    // search and place
    void * bp;
    void * place_res;
    if ((bp = find_fit(aligned_block_size)) != NULL){
        #if defined(DEBUG_MALLOC) || defined(DEBUG) 
            printf("DEBUG [mm_malloc][find_fit]: placing size %d at %p\n", size, bp);
        #endif
        place_res = place(bp, aligned_block_size);
        #if defined(DEBUG_MALLOC) || defined(DEBUG) 
            show_list();
        #endif
        return place_res;
    }

    if ((bp = extend_heap(extend_size/WSIZE)) == NULL)
        return NULL;
    #if defined(DEBUG_MALLOC) || defined(DEBUG) 
        printf("DEBUG [mm_malloc][extend_heap]: placing size %d at %p\n", size, bp);
    #endif
    place_res = place(bp, aligned_block_size);
    #if defined(DEBUG_MALLOC) || defined(DEBUG) 
        show_list();
    #endif
    return place_res;
}

static void * find_fit(size_t size)
{
    for (void * bp = list_head; bp != NULL; bp=NEXT_BLKP(bp)) 
    {
        if ((!GET_BLK_ALLOC(bp)) && (GET_BLK_SIZE(bp) >= size))
        {
            #if defined(DEBUG_FIND_FIT) || defined(DEBUG) 
                printf("DEBUG [find_fit]: find fit finished. First-Fit bp is %p\n", bp);
            #endif
            return bp;
        }
    }
    
    return NULL;
}

static void * place(void * bp, size_t size)
{
    // calc size
    size_t curr_size = GET_BLK_SIZE(bp);
    #if defined(DEBUG_PLACE) || defined(DEBUG) 
        printf("DEBUG [place]: overall size is %d\n", curr_size);
    #endif

    // reserve vars
    void * prev=PREV_BLKP(bp), * next=NEXT_BLKP(bp);

    // no need to split
    if (curr_size - size < FRAG_SIZE*WSIZE) {
        #if defined(DEBUG_PLACE) || defined(DEBUG) 
            printf("DEBUG [place][no-split]: placed right away\n");
        #endif
        PUT(HDRP(bp), PACK(curr_size, 1));
        PUT(FD(prev), next);
        PUT(BK(next), prev);
        return bp; 
    }
    
    // split the block
    PUT(HDRP(bp), PACK(curr_size-size, 0));
    PUT(FTRP(bp), PACK(curr_size-size, 0));
    #if defined(DEBUG_PLACE) || defined(DEBUG) 
        printf("DEBUG [place][split]: split part start at %p, size is %d\n", bp, curr_size-size);
    #endif

    void * alloc_bp = FTRP(bp)+2*WSIZE;
    PUT(HDRP(alloc_bp), PACK(size, 1));
    #if defined(DEBUG_PLACE) || defined(DEBUG) 
        printf("DEBUG [place][split]: alloc start at %p, size is %d\n", alloc_bp, size);
    #endif

    coalesce(bp);
    return alloc_bp;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    #if defined(DEBUG_FREE) || defined(DEBUG) 
        printf("DEBUG [mm_free]: freeing %p\n", bp);
    #endif

    // mark bp as free, create FTR
    PUT(HDRP(bp), PACK(GET_BLK_SIZE(bp), 0));
    PUT(FTRP(bp), PACK(GET_BLK_SIZE(bp), 0));

    // locate bp, link with prev and next
    void * curr, * prev;
    for (curr=list_head; curr != NULL; curr=NEXT_BLKP(curr)){
        if (curr > bp) break;    
    }
    if (curr == NULL){
        #if defined(DEBUG_FREE) || defined(DEBUG) 
            printf("DEBUG [mm_free][locate bp]: ERROR! curr is NULL, which means bp is higher than tail\n");
        #endif
    }
    #if defined(DEBUG_FREE) || defined(DEBUG) 
        printf("DEBUG [mm_free][loc-curr]: %p -> %p -> %p\n",PREV_BLKP(curr), bp, curr);
    #endif
    prev = PREV_BLKP(curr);

    // link prev, bp, curr
    PUT(FD(prev), bp);
    PUT(BK(bp), prev);
    PUT(FD(bp), curr);
    PUT(BK(curr), bp);

    // coalesce
    coalesce(bp);
    #if defined(DEBUG_FREE) || defined(DEBUG) 
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

    size_t req_size_aligned = ALIGN_TO_FULLBLK(req_size);
    size_t curr_size = GET_BLK_SIZE(bp);
    #if defined(DEBUG_REALLOC) || defined(DEBUG) 
        printf("DEBUG [mm_realloc]: original size %d, aligned to %d, current block size is %d\n", 
               req_size, 
               req_size_aligned, 
               curr_size);
    #endif

    if (req_size_aligned > curr_size)
    {
        void * res;
        res = mm_realloc_req_greater_block(bp, req_size_aligned);
        return res;
    }

    else if (req_size_aligned == curr_size)
    {
        #if defined(DEBUG_REALLOC) || defined(DEBUG) 
            printf("DEBUG [mm_realloc][eq]: do nothing\n");
        #endif
        return bp;
    }

    else
    {
        void * res;
        res = mm_realloc_req_smaller_block(bp, req_size_aligned);
        return res;
    }

}

static void * mm_realloc_req_greater_block(void * bp, size_t aligned_req_size)
{
    // try to find a coherent next free block
    void * curr = list_head;
    for (; curr; curr=NEXT_BLKP(curr)) if (curr > bp) break;

    // case 1: there is an adjacent free block with inadequate size
    if ((bp+GET_BLK_SIZE(bp) == curr) && (GET_BLK_SIZE(bp)+GET_BLK_SIZE(curr)) >= aligned_req_size)
    {
        #if defined(DEBUG_REALLOC) || defined(DEBUG) 
            printf("DEBUG [mm_realloc][gg-in-place]: found curr (%p), adjacent to bp (%p)\n", curr, bp); 
        #endif

        // calc rest space
        size_t overall_size = GET_BLK_SIZE(bp) + GET_BLK_SIZE(curr); 
        size_t split_size = overall_size - aligned_req_size;

        // case 1.1: split is too small (less than 6*WORDS), do not split
        // connect prev and next
        if (split_size < FRAG_SIZE*WSIZE){
            void * prev = PREV_BLKP(curr), * next = NEXT_BLKP(curr);
            PUT(FD(prev), next);
            PUT(BK(next), prev);

            PUT(HDRP(bp), PACK(overall_size, 1));
            #if defined(DEBUG_REALLOC) || defined(DEBUG) 
                printf("DEBUG [mm_realloc][gg-in-place]: do not split\n"); 
            #endif
            return bp;
        }

        // case 1.2: split 
        else{
            void * prev = PREV_BLKP(curr), * next = NEXT_BLKP(curr);
            PUT(HDRP(bp), aligned_req_size);
            void * split = (void *)((char *)(bp) + GET_BLK_SIZE(bp));
            PUT(HDRP(split), PACK(split_size, 0));
            PUT(FTRP(split), PACK(split_size, 0));
            
            PUT(FD(prev), split);
            PUT(FD(split), next);
            PUT(BK(next), split);
            PUT(BK(split), prev);
            
            #if defined(DEBUG_REALLOC) || defined(DEBUG) 
                printf("DEBUG [mm_realloc][gg-in-place]: split to %p, coalesce now\n", split); 
            #endif

            coalesce(split);
            return bp;
        }
    }

    // case 2: adjacent but too small; no adjacent free block -> just malloc, memcpy, free
    else{

        void *oldbp = bp;
        void *newbp;
        size_t copySize;

        copySize = GET_BLK_SIZE(oldbp) - WSIZE;
        newbp = mm_malloc(aligned_req_size);
        memcpy(newbp, oldbp, copySize);
        mm_free(oldbp);
        #if defined(DEBUG_REALLOC) || defined(DEBUG) 
            printf("DEBUG [mm_realloc][gg-memcpy]: copy to %p\n", newbp); 
        #endif
        return newbp;
    }
}

static void * mm_realloc_req_smaller_block(void * bp, size_t req_size_aligned)
{
    size_t split_size = GET_BLK_SIZE(bp) - req_size_aligned;

    // case 1
    if (split_size < FRAG_SIZE * WSIZE)
    {
        #if defined(DEBUG_REALLOC) || defined(DEBUG) 
            printf("DEBUG [mm_realloc][ll-cannot-split]: do nothing\n"); 
        #endif
        return bp;
    }

    else
    {
        #if defined(DEBUG_REALLOC) || defined(DEBUG) 
            printf("DEBUG [mm_realloc][ll-can-split]: actually i can split, but i must protect redundant bytes\n"); 
        #endif
        return bp;
    }
}


void show_list()
{
    void * bp = list_head;
        printf("----------------- HEAP LIST -----------------\n");
        printf("DEBUG [show list]: %-10s \t ALLOC \t SIZE   \t GET_HDR\n", "BP");

    for (; bp; bp=NEXT_BLKP(bp)) 
    {
        printf("DEBUG [show list]: %-p \t %-5d \t %-6d \t %d\n", bp, (int)(GET_ALLOC(HDRP(bp))), (uint)(GET_SIZE(HDRP(bp))), GET(HDRP(bp)));
        // printf("DEBUG [show list]: %-p \t %-5d \t %-6d \t %d\n", bp, (int)(GET_ALLOC(HDRP(bp))), (uint)(GET_SIZE(HDRP(bp))), GET(HDRP(bp)));
    }
        
    printf("---------------------------------------------\n");
}
