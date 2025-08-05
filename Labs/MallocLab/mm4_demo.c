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

/*********************************************************
 * Global vars
 ********************************************************/
void *list_16,   *list_32,   *list_64,  *list_128;
void *list_256, *list_512, *list_1024, *list_2048, *list_4096, *list_8192, *list_jumbo;
void *heap_head;
void *prologue;
void *epilogue;
#define LIST_NUM                11
#define LIST_MAX_INDEX          (LIST_NUM-1)
#define BLOCK_NUM_LIMIT         4
#define INIT_LIST(list, init, index)   do {list=init+index*WSIZE; PUT(list, 0);} while (0)

static void *get_free_block(size_t aligned_size);
static void detach_block(void *bp);
static void *extend_heap(size_t aligned_size);
static uint _get_list_index(size_t aligned_size);
static void *get_list(size_t aligned_size);
static void *place(void *bp, size_t aligned_size);
static void join_in_list(void *bp);
static void coalesce(void *bp);
static void show_list(void *);


int mm_init(void){
    void* tmp;
    if ((tmp = mem_sbrk((LIST_NUM+3)*WSIZE)) == (void *)-1)
        return -1;

    /*  extend 11+3 WSIZE
    
        |   |   |   |   |   |   |   |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |-->  11  headers  <--|   |
                              |   |--> epilogue
                              |
                              |--> prologue                     
    */

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

    // prologue, epilogue
    PUT(tmp+11*WSIZE, PACK(8, 1));
    PUT(tmp+12*WSIZE, PACK(8, 1));
    PUT(tmp+13*WSIZE, PACK(0, 1));
    epilogue = tmp + 13*WSIZE;

    #if defined(DEBUG) || defined(DEBUG_MM_INIT)
        printf("\n[DEBUG][mm_init] list_16\t@ %p\n", list_16);
        printf("[DEBUG][mm_init] list_32\t@ %p\n", list_32);
        printf("[DEBUG][mm_init] list_64\t@ %p\n", list_64);
        printf("[DEBUG][mm_init] list_128\t@ %p\n", list_128);
        printf("[DEBUG][mm_init] list_256\t@ %p\n", list_256);
        printf("[DEBUG][mm_init] list_512\t@ %p\n", list_512);
        printf("[DEBUG][mm_init] list_1024\t@ %p\n", list_1024);
        printf("[DEBUG][mm_init] list_2048\t@ %p\n", list_2048);
        printf("[DEBUG][mm_init] list_4096\t@ %p\n", list_4096);
        printf("[DEBUG][mm_init] list_8192\t@ %p\n", list_8192);
        printf("[DEBUG][mm_init] list_jumbo\t@ %p\n", list_jumbo);
        printf("[DEBUG][mm_init] epilogue\t@ %p\n", epilogue);

    #endif
    
    return 0;
}

static uint _get_list_index(size_t aligned_size)
{
    uint size = (uint)(aligned_size-1);
    uint res = 0;

    uint r16 = size & 0xFFFF;
    uint f16 = (size>>16) & 0xFFFF;
    uint flag_f16 = !(!(f16));
    res += (flag_f16<<4);
    uint a16 = (f16 & (~(flag_f16-1))) + (r16 & (flag_f16-1));

    uint r8 = a16 & 0xFF;
    uint f8 = (a16>>8) & 0xFF;
    uint flag_f8 = !(!(f8));
    res += (flag_f8<<3);
    uint a8 = (f8 & (~(flag_f8-1))) + (r8 & (flag_f8-1));

    uint r4 = a8 & 0xF;
    uint f4 = (a8>>4) & 0xF;
    uint flag_f4 = !(!(f4));
    res += (flag_f4<<2);
    uint a4 =  (f4 & (~(flag_f4-1))) + (r4 & (flag_f4-1));

    uint r2 = a4 & 0x3;
    uint f2 = (a4>>2) & 0x3;
    uint flag_f2 = !(!(f2));
    res += (flag_f2<<1);
    uint a2 =  (f2 & (~(flag_f2-1))) + (r2 & (flag_f2-1));

    uint r1 = a2 & 0x1;
    uint f1 = (a2>>1) & 0x1;
    uint flag_f1 = !(!(f1));
    res += flag_f1;
    uint a1 =  (f1 & (~(flag_f1-1))) + (r1 & (flag_f1-1));
    res += a1;

    res = MIN(LIST_MAX_INDEX, res-4);
    #if defined(DEBUG) || defined(DEBUG__GET_LIST_INDEX)
        printf("[DEBUG][get-list-index] size %d ->, index %d: [%d, %d]\n", aligned_size, res, 1<<(res+3), 1<<(res+4));
    #endif
    return res;
}

static void *get_free_block(size_t aligned_size)
{
    size_t index = _get_list_index(aligned_size);
    void *curr;
    void *list_head;
    void *res = NULL;

    for (int i=index; i<=LIST_MAX_INDEX; i++)
    {
        list_head = heap_head+WSIZE*i;

        #if defined(DEBUG) || defined(DEBUG_GET_FREE_BLOCK)
            printf("[DEBUG][get-free-block] start to find free block in list [%d, %d]\n", 1<<(i+3), 1<<(i+4));
            show_list(list_head);
        #endif

        curr = GET(list_head);
        if (!curr) continue; // this list is empty

        for (; curr; curr=FD(curr))
            if (GET_BLK_SIZE(curr) >= aligned_size) {res = curr; break;}
    }

    #if defined(DEBUG) || defined(DEBUG_GET_FREE_BLOCK)
        printf("[DEBUG][get-free-block] result is %p\n", res);
    #endif
    if (res) detach_block(res);
    return res;
}

static void detach_block(void *bp)
{
    void *bk = BK(bp); 
    void *fd = FD(bp);
    void *list_head = get_list(GET_BLK_SIZE(bp));

    #if defined(DEBUG) || defined(DEBUG_DETACH_BLOCK)
        printf("[DEBUG][detach_block] detach block %p\n", bp);
        show_list(list_head);
    #endif

    if (!bk) { 
        #if defined(DEBUG) || defined(DEBUG_DETACH_BLOCK)
            printf("[DEBUG][detach_block] bk is [%p], list_head [%p] should points to fd [%p]\n", bk, list_head, fd);
        #endif
        PUT(list_head, fd); 
    }    // bk is empty, this is the first block in list
    else { 
        #if defined(DEBUG) || defined(DEBUG_DETACH_BLOCK)
            printf("[DEBUG][detach_block] bk is a block [%p], bk.FDP should points to fd [%p]\n", bk, fd);
        #endif
        PUT(FDP(bk), fd); 
    }        // bk is another block

    if (fd) { 
        #if defined(DEBUG) || defined(DEBUG_DETACH_BLOCK)
            printf("[DEBUG][detach_block] fd is a block [%p], fd.BKP should points to bk [%p]\n", fd, bk);
        #endif
        PUT(BKP(fd), bk); 
    }       // if fd: there is another block, otherwise bp is the last block
    #if defined(DEBUG) || defined(DEBUG_DETACH_BLOCK)
        printf("[DEBUG][detach_block][after]\n");
        show_list(list_head);
    #endif
    SET_FREE(bp);
}

static void *extend_heap(size_t aligned_size)
{
    #if defined(DEBUG) || defined(DEBUG_EXTEND_HEAP)
        printf("[DEBUG][extend-heap] extend size: %d\n", aligned_size);
    #endif

    // bp=sbrk, fill HDR FTR
    void * bp;
    if ((bp = mem_sbrk(aligned_size)) == (void *)-1)
        return NULL;

    PUT(bp+aligned_size-WSIZE, PACK(0, 1)); // re-align epilogue
    #if defined(DEBUG) || defined(DEBUG_EXTEND_HEAP)
        printf("epilogue: %p\n", GET(bp+aligned_size-WSIZE));
    #endif
    PUT(HDRP(bp), PACK(aligned_size, 0));   // put header
    PUT(FTRP(bp), PACK(aligned_size, 0));   // put footer
    join_in_list(bp);
    return bp;
}



static void *get_list(size_t aligned_size)
{
    return heap_head + WSIZE * _get_list_index(aligned_size);
}

void *place(void *bp, size_t aligned_size){
    size_t block_size = GET_BLK_SIZE(bp);
    size_t split_size = block_size - aligned_size;
    void * list_head = get_list(block_size);
    #if defined(DEBUG) || defined(DEBUG_PLACE)
        printf("[DEBUG][place] curr block %d, aligned %d, split size %d\n", block_size, aligned_size, split_size);
    #endif

    // decide which list should split-block be in
    // case 1: cannot split
    if (split_size < 4 * WSIZE){
        #if defined(DEBUG) || defined(DEBUG_PLACE)
            printf("[DEBUG][place][case 1] cannot split\n");
        #endif
        detach_block(bp);
        SET_ALLOC(bp); 
        
        #if defined(DEBUG)
        printf("SPECIAL INSERT: header[%p] %p, footer[%p] %p\n", HDRP(bp), GET(HDRP(bp)), FTRP(bp), GET(FTRP(bp)));
        #endif
        
        
        return bp;
    }

    // case 2: commence split; split-block and curr-block are in the same list, so ** ALLOC THE TAIL OF CURR-BLOCK **
    void *split_head = get_list(split_size);
    if (split_head == list_head)
    {
        #if defined(DEBUG) || defined(DEBUG_PLACE)
            printf("[DEBUG][place][case 2] split is in the same list, alloc block tail\n");
        #endif
        PUT(HDRP(bp), PACK(split_size, 0));
        PUT(FTRP(bp), PACK(split_size, 0));

        void *res = NEXT(bp);
        PUT(HDRP(res), PACK(aligned_size, 1));
        PUT(FTRP(res), PACK(aligned_size, 1));
        return res;
    }

    // case 3: commence split; split-block in another list, ** ALLOC HEAD OF CURR-BLOCK **
    #if defined(DEBUG) || defined(DEBUG_PLACE)
        printf("[DEBUG][place][case 3] split in diff list\n");
    #endif
    detach_block(bp);
    PUT(HDRP(bp), PACK(aligned_size, 1));
    PUT(FTRP(bp), PACK(aligned_size, 1));

    void *split = NEXT(bp);
    PUT(HDRP(split), PACK(split_size, 0));
    PUT(FTRP(split), PACK(split_size, 0));
    join_in_list(split);
    return bp;
}

void join_in_list(void *bp)
{
    void *list = get_list(GET_BLK_SIZE(bp));
    #if defined(DEBUG) || defined(DEBUG_JOIN_IN_LIST)
        printf("[DEBUG][join-in-list] bp %p size %d, list [%d, %d]:\n", bp, GET_BLK_SIZE(bp), RANGE_LO(list), RANGE_HI(list));
        printf("[DEBUG][join-in-list] pre:\n");
        show_list(list);
    #endif

    // join the list by addr, not lifo
    // find if there is a block has bigger addr
    void *prev = NULL; 
    void *curr = GET(list);
    for (; curr; prev=curr, curr=FD(curr))
    {
        if (curr > bp) break;
    }
    #if defined(DEBUG) || defined(DEBUG_JOIN_IN_LIST)
        printf("[DEBUG][join-in-list] list %p points to %p , curr (the next bigger addr block) is %p, prev is %p\n", list, GET(list), curr, prev);
    #endif

    /*
        case 1: prev is NULL, curr is NULL: this list is empty
        case 2: prev is NULL, curr is not : join the list head
        case 4: prev is not,  curr is not : insert at the middle
        case 4: prev is not,  curr is NULL: append at the list tail
    */

    // case 1: list is empty
    if ((!prev) && (!curr)){
        PUT(list, bp);
        PUT(FDP(bp), 0);
        PUT(BKP(bp), 0);}

    // case 2: insert at head
    else if ((!prev) && (curr)){
        PUT(list, bp);
        PUT(BKP(bp), 0);
        PUT(FDP(bp), curr);
        PUT(BKP(curr), bp);
    }

    // case 3: insert at mid
    else if (prev && curr) {
        PUT(FDP(prev), bp);
        PUT(BKP(bp), prev);
        PUT(FDP(bp), curr);
        PUT(BKP(curr), bp);
    }

    // case 4: insert at tail
    else {
        PUT(FDP(prev), bp);
        PUT(BKP(bp), prev);
        PUT(FDP(bp), 0);
    }
    #if defined(DEBUG) || defined(DEBUG_JOIN_IN_LIST)
        printf("[DEBUG][join-in-list] joined, before coalesce:\n");
        show_list(list);
    #endif

    coalesce(bp);
    #if defined(DEBUG) || defined(DEBUG_JOIN_IN_LIST)
        printf("[DEBUG][join-in-list] after coalesce:\n");
        show_list(list);
    #endif
}

void coalesce(void *bp){
    void *prev = PREV(bp);
    void *next = NEXT(bp);
    size_t original_size = GET_BLK_SIZE(bp);

    int prev_is_alloc = GET_BLK_ALLOC(prev);
    int next_is_alloc = GET_BLK_ALLOC(next);

    #if defined(DEBUG) || defined(DEBUG_COALESCE)
        printf("      address     alloc size  header  footer\n");
        printf("prev %p %d %d %p %p\n", prev, GET_BLK_ALLOC(prev), GET_BLK_SIZE(prev), GET(HDRP(prev)), GET(FTRP(prev)));
        printf("curr %p %d %d %p %p\n", bp  , GET_BLK_ALLOC( bp ), GET_BLK_SIZE( bp ), GET(HDRP( bp )), GET(FTRP( bp )));
        printf("next %p %d %d %p %p\n", next, GET_BLK_ALLOC(next), GET_BLK_SIZE(next), GET(HDRP(next)), GET(FTRP(next)));
        printf("prev footer [%p] %p\n", bp-DSIZE, GET(bp-DSIZE));
    #endif

    // case 1: do not merge
    if ((prev_is_alloc) && (next_is_alloc)) { return; }


    size_t overall_size = 0;
    // case 2: merge prev
    if ((!prev_is_alloc) && (next_is_alloc))
    {
        overall_size = GET_BLK_SIZE(bp) + GET_BLK_SIZE(prev);
        detach_block(prev);
        PUT(HDRP(prev), PACK(overall_size, 0));
        PUT(FTRP(prev), PACK(overall_size, 0));
        join_in_list(prev);
    }

    // case 3: merge next
    else if ((prev_is_alloc) && (!next_is_alloc)){
        overall_size = GET_BLK_SIZE(bp) + GET_BLK_SIZE(next);
        detach_block(next);
        PUT(HDRP(bp), PACK(overall_size, 0));
        PUT(FTRP(bp), PACK(overall_size, 0));
        join_in_list(bp);
    }

    // case 4: merge prev + next
    else if ((!prev_is_alloc) && (!next_is_alloc)){
        overall_size = GET_BLK_SIZE(prev) + GET_BLK_SIZE(bp) + GET_BLK_SIZE(next);
        detach_block(prev);
        detach_block(next);
        PUT(HDRP(prev), PACK(overall_size, 0));
        PUT(FTRP(prev), PACK(overall_size, 0));
        join_in_list(prev);
    }
    
    #if defined(DEBUG) || defined(DEBUG_COALESCE)
        printf("[DEBUG][coalesce] block size %d -> %d\n", original_size, overall_size);
    #endif

}

void *mm_malloc(size_t size){
    size_t aligned_size = ALIGN_TO_FULLBLK(size);
    #if defined(DEBUG) || defined(DEBUG_MM_MALLOC)
        printf("[DEBUG][mm_malloc][align block size] %6d -> %6d\n", size, aligned_size);
    #endif

    void *bp = get_free_block(aligned_size);
    #if defined(DEBUG) || defined(DEBUG_MM_MALLOC)
        if (bp == NULL)
        {
            printf("[DEBUG][mm_malloc][dispatch list] extend now\n");
        }
        else{
            void *_tmp_list_head = get_list(GET_BLK_SIZE(bp));
            printf("[DEBUG][mm_malloc][dispatch list] ready to place in %p (list [%d, %d] )\n", _tmp_list_head, RANGE_LO(_tmp_list_head), RANGE_HI(_tmp_list_head));
        }
        
    #endif
    if (bp == NULL) bp = extend_heap(aligned_size);
    if (bp == NULL) return NULL;
    
    return place(bp, aligned_size);
}

void mm_free(void *bp){
    SET_FREE(bp);
    #if defined(DEBUG) || defined(DEBUG_FREE)
        printf("[DEBUG][mm-free] freeing block %p, size %d, joining now\n", bp, GET_BLK_SIZE(bp));
    #endif
    join_in_list(bp);
}

void *mm_realloc(void *bp, size_t req_size){
    size_t req_size_aligned = ALIGN_TO_FULLBLK(req_size);
    size_t bp_size = GET_BLK_SIZE(bp);

    // case 1: request smaller size: do nothing
    if (req_size_aligned <= bp_size) return bp;


    // case 2: request bigger size
    // case 2.1: bp is at heap tail
    // case 2.2：can find a free block somewhere, detach it 
    // case 2.3: alloc, memcpy, free
    void *oldbp = bp;
    void *newbp;
    size_t copySize;

    copySize = GET_BLK_SIZE(oldbp) - WSIZE;
    newbp = mm_malloc(req_size_aligned);
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
        printf("                         size(hdrp)    fdp    bkp    ftrp\n");
        while ((bp) && (GET_BLK_SIZE(bp))){
            printf("[%p]->[%p]: %6d \t %p \t %p \t %p\n", bp, bp+GET_BLK_SIZE(bp), GET_BLK_SIZE(bp), GET(FDP(bp)), GET(BKP(bp)), GET(FTRP(bp)));
            bp = FD(bp);
        }
    }

    printf("--------------------------------------------\n");
}