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
#define ALIGN_TO_FULLBLK(size)      (MAX((((size+3) & ~0x7) + DSIZE), 4*WSIZE))

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
#define FDP(bp)             (bp)
#define BKP(bp)             ((char *)(bp) + WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_SIZE(bp) - DSIZE)
#define GET_SIZE(bp)        ((size_t)(GET(HDRP(bp))))
#define NEXT(bp)            (GET(FDP(bp)))
#define PREV(bp)            (GET(BKP(bp)))


/*********************************************************
 * Global vars
 ********************************************************/
void *list_16,   *list_32,   *list_64,  *list_128;
void *list_256, *list_512, *list_1024, *list_2048, *list_4096, *list_8192, *list_jumbo;
void *heap_head;
#define LIST_NUM                11
#define LIST_MAX_INDEX          (LIST_NUM-1)
#define BLOCK_NUM_LIMIT         4
#define INIT_LIST(list, init, index)   do {list=init+index*WSIZE; PUT(list, 0);} while (0)

void *get_free_block(size_t aligned_size);
void *detach_block(void *bp);
void *extend_heap(size_t aligned_size);
uint _get_list_index(size_t aligned_size);
void *get_list_by_size(size_t aligned_size);
void *place(void *bp, size_t aligned_size);
void join_in_list(void *bp);
void coalesce(void *bp);
void show_list(void *);


int mm_init(void){
    void* tmp;
    if ((tmp = mem_sbrk((LIST_NUM+1)*WSIZE)) == (void *)-1)
        return -1;

    // headers
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
    PUT(tmp+11*WSIZE, 0);  // epilogue

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
    #endif
    
    return 0;
}

void *get_free_block(size_t aligned_size)
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

        if (!GET(list_head)) continue; // this list is empty

        for (curr=GET(list_head); curr; curr=NEXT(curr))
            if (GET_SIZE(curr) >= aligned_size) {res = curr; break;}
    }

    #if defined(DEBUG) || defined(DEBUG_GET_FREE_BLOCK)
        printf("[DEBUG][get-free-block] result is %p\n", res);

    #endif

    return res;
}

void *detach_block(void *bp)
{
    #if defined(DEBUG) || defined(DEBUG_DETACH_BLOCK)
            printf("[DEBUG][detach_block] detach block %p\n", bp);
            show_list(GET(FTRP(bp)));
    #endif
    void *prev = PREV(bp); 
    void *next = NEXT(bp);

    // prev is empty, this is the first block in list
    if (!prev){
        PUT(GET(FTRP(bp)), next); // GET(FTRP(bp)) is list_head
    }
    else{
        PUT(FDP(prev), next);     // prev is another block
    }

    // if next: there is another block
    // else:    bp is the last block
    if (next) { PUT(BKP(next), prev); }
    return bp;
}

void *extend_heap(size_t aligned_size)
{
    #if defined(DEBUG) || defined(DEBUG_EXTEND_HEAP)
        printf("[DEBUG][extend-heap] extend size: %d\n", aligned_size);
    #endif

    // bp=sbrk, fill HDR FTR
    void * bp;
    if ((bp = mem_sbrk(aligned_size)) == (void *)-1)
        return NULL;

    PUT(bp+aligned_size-WSIZE, 0); // epilogue
    PUT(HDRP(bp), aligned_size);
    PUT(FTRP(bp), 0);
    return bp;
}

uint _get_list_index(size_t aligned_size)
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
        printf("[DEBUG][get-list-index] size %d -> [%d, %d]\n", aligned_size, 1<<(res+3), 1<<(res+4));
    #endif
    return res;
}

void *get_list_by_size(size_t aligned_size)
{
    return heap_head + WSIZE * _get_list_index(aligned_size);
}

void *place(void *bp, size_t aligned_size){
    // get_free_block 返回一个在列表中正在连接中的block, 其HDR是尺寸, FTR是list head指针
    // extend_heap 返回一个未连接的block, 除了HDR什么都没有
    size_t block_size = GET_SIZE(bp);
    size_t split_size = block_size - aligned_size;
    void * block_head = GET(FTRP(bp));
    #if defined(DEBUG) || defined(DEBUg_PLACE)
        printf("[DEBUG][place] curr block %d, aligned %d, split size %d\n", block_size, aligned_size, split_size);
    #endif

    // 查看split_size应该属于哪个list
    // 无法分割, 不属于任何list
    if (split_size < 16){
        #if defined(DEBUG) || defined(DEBUg_PLACE)
            printf("[DEBUG][place][case 1] cannot split\n");
        #endif
        if (block_head) detach_block(bp);
        return bp;
    }

    // 可分割，并且分割块和当前块位于同一个list，那就直接分配当前块的后半部分
    void *split_head = get_list_by_size(split_size);
    if (split_head == block_head)
    {
        #if defined(DEBUG) || defined(DEBUg_PLACE)
            printf("[DEBUG][place][case 2] split is in the same list, alloc block tail\n");
        #endif
        PUT(HDRP(bp), split_size);
        PUT(FTRP(bp), block_head);
        void *res = bp + split_size;
        PUT(HDRP(res), aligned_size);
        return res;
    }

    // 可分割，并且分割块和当前块位于不同list
    #if defined(DEBUG) || defined(DEBUg_PLACE)
        printf("[DEBUG][place][case 3] split in diff list\n");
    #endif
    if (block_head) detach_block(bp);
    // split加入list
    void *split = bp + aligned_size;
    PUT(HDRP(split), split_size);
    PUT(FTRP(split), split_head);
    join_in_list(split);

    // 返回bp
    PUT(HDRP(bp), aligned_size);
    return bp;
}

void join_in_list(void *bp)
{
    void *list = GET(FTRP(bp));
    #if defined(DEBUG) || defined(DEBUG_JOIN_IN_LIST)
        printf("[DEBUG][join-in-list] bp %p size %d, list [%d, %d]:\n", bp, GET_SIZE(bp), (1<<((list-heap_head)/WSIZE+3)), 1<<((list-heap_head)/WSIZE+4));
        printf("[DEBUG][join-in-list] pre:\n");
        show_list(list);
    #endif
    void *prev = NULL; 
    void *curr = GET(list);
    for (; curr; prev=curr, curr=NEXT(curr))
    {
        if (curr > bp) break;
    }

    
    // case 1: list is empty
    if ((!prev) && (!curr))
    {
        PUT(list, bp);
        PUT(FDP(bp), 0);
        PUT(BKP(bp), 0);
    }

    // case 2.1: insert at head
    else if ((!prev) && (curr)){
        PUT(list, bp);
        PUT(BKP(bp), 0);
        PUT(FDP(bp), curr);
        PUT(BKP(curr), bp);
        coalesce(bp);
    }

    // case 2.2: insert at mid
    else if (prev && curr) {
        PUT(FDP(prev), bp);
        PUT(BKP(bp), prev);
        PUT(FDP(bp), curr);
        PUT(BKP(curr), bp);
        coalesce(bp);
    }

    // case 2.3: insert at tail
    else {
        PUT(FDP(prev), bp);
        PUT(BKP(bp), prev);
        PUT(FDP(bp), 0);
        coalesce(bp);
    }
    #if defined(DEBUG) || defined(DEBUG_JOIN_IN_LIST)
        printf("[DEBUG][join-in-list] post:\n");
        show_list(list);
    #endif

}

void coalesce(void *bp){
    void *prev = PREV(bp);
    void *next = NEXT(bp);
    size_t original_size = GET_SIZE(bp);

    int flag_merge_prev = 0;
    if ((prev) && ((prev + GET_SIZE(prev)) == bp))
        flag_merge_prev = 1;

    int flag_merge_next = 0;
    if ((next) && ((bp + GET_SIZE(bp)) == next))
        flag_merge_next = 1;

    // case 1: do not merge
    if ((!flag_merge_prev) && (!flag_merge_next)) { return; }


    size_t overall_size;
    // case 2: merge prev
    if ((flag_merge_prev) && (!flag_merge_next))
    {
        overall_size = GET_SIZE(bp) + GET_SIZE(prev);
        PUT(HDRP(prev), overall_size);
        PUT(FDP(prev), next);
        if (next) PUT(BKP(next), prev);
        bp = prev;
    }

    // case 3: merge next
    else if ((!flag_merge_prev) && (flag_merge_next)){
        overall_size = GET_SIZE(bp) + GET_SIZE(next);
        PUT(HDRP(bp), overall_size);
        PUT(FDP(bp), NEXT(next));
        if (NEXT(next)) PUT(BKP(NEXT(next)), bp);
    }

    // case 4: merge prev + next
    else if (flag_merge_prev && flag_merge_next){
        overall_size = GET_SIZE(prev) + GET_SIZE(bp) + GET_SIZE(next);
        PUT(HDRP(prev), overall_size);
        PUT(FDP(prev), NEXT(next));
        if (NEXT(next)) PUT(BKP(NEXT(next)), prev);
        bp = prev;
    }
    
    // case 2-4: check overall_size
    void *original_list = get_list_by_size(original_size);
    void *new_list = get_list_by_size(overall_size);
    #if defined(DEBUG) || defined(DEBUG_COALESCE)
        printf("[DEBUG][coalesce] block size %d -> %d\n", original_size, overall_size);
    #endif


    if (original_list == new_list){
        return;
    }
    #if defined(DEBUG) || defined(DEBUG_COALESCE)
        printf("[DEBUG][coalesce] detach and join in new list\n");
    #endif
    
    bp = detach_block(bp);
    PUT(FTRP(bp), new_list);
    join_in_list(bp);


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
            void *_tmp_list_head = GET(FTRP(bp));
            size_t _tmp_list_size = 1 << ((_tmp_list_head-heap_head)/WSIZE+4);

            printf("[DEBUG][mm_malloc][dispatch list] ready to place in %p (list [%d, %d] )\n", _tmp_list_head, _tmp_list_size>>1, _tmp_list_size);
        }
        
    #endif
    if (bp == NULL) bp = extend_heap(aligned_size);
    if (bp == NULL) return NULL;
    
    return place(bp, aligned_size);
}

void mm_free(void *bp){
    void *list;
    list = get_list_by_size(GET_SIZE(bp));
    PUT(FTRP(bp), list);
    #if defined(DEBUG) || defined(DEBUG_FREE)
        printf("[DEBUG][mm-free] freeing block %p, size %d, joining now\n", bp, GET_SIZE(bp));
    #endif
    join_in_list(bp);
}

void *mm_realloc(void *bp, size_t req_size){
    size_t req_size_aligned = ALIGN_TO_FULLBLK(req_size);
    size_t bp_size = GET_SIZE(bp);

    // case 1: request smaller size: do nothing
    if (req_size_aligned <= bp_size) return bp;


    // case 2: request bigger size
    // case 2.1: bp is at heap tail
    // case 2.2：can find a free block somewhere, detach it 
    // case 2.3: alloc, memcpy, free
    void *oldbp = bp;
    void *newbp;
    size_t copySize;

    copySize = GET_SIZE(oldbp) - WSIZE;
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
        while (bp){
            printf("[%p]->[%p]: %6d \t %p \t %p \t %p\n", bp, bp+GET_SIZE(bp), GET_SIZE(bp), GET(FDP(bp)), GET(BKP(bp)), GET(FTRP(bp)));
            bp = NEXT(bp);
        }
    }

    printf("--------------------------------------------\n");
}