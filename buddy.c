#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> //uint8_t ,uint64_t
#include "buddy.h"

#define TAG(n) (((BlockHeader*)n)->tag)
#define KVAL(n) (((BlockHeader*)n)->kval)
#define DATA(n) (void*)(((void**)(n))+1)
#define HEAD(n) (void*)(((void**)(n))-1)

#define LINKF(n) (*(void**)DATA(n))
#define LINKB(n) (*((void**)DATA(n)+1))

#define NEXT_BLOCK(n,k) n + (1 << (k))
#define PREV_BLOCK(n,k) n - (1 << (k))

typedef struct block_header{
	uint8_t tag;
	uint8_t kval;
} BlockHeader;

static void** available[MAX_CHUNK_SIZE + 1] = {NULL};

static void* start_pointer;
static void* end_pointer;

static const int PTR_SIZE = sizeof(void*);

//HELPERS


/*
 * Returns logarithm of x in base 2 
 */
uint8_t log2i(uint64_t x)
{
	uint8_t k = 0;
	while (x > 0)
	{
		k++;
		x = x >> 1;
	}
	return k-1;
}

/*
 * Finds minimum block size (2^k) required for an object with given size
 */
uint8_t block_size(uint64_t size)
{
	size = size + PTR_SIZE;
	uint8_t level = log2i(size);
	
	//if (1 << level < size)
		//level = level + 1;
	
	if (level < MIN_REQUEST_SIZE)
		return MIN_REQUEST_SIZE;
	return level;
}

/*
 * Returns the pointer of the buddy of given block
 * 
 */
void* buddy(void* block)
{
	int k = KVAL(block);
	
	if (((block - start_pointer) >> k) % 2 == 0)
	{
		return block + (1 << k);
	}
	else
		return block - (1 << k);
}



void* pop_block(uint8_t k)
{
	void* block = available[k];
	if (block == NULL)
		return NULL;
	
	//Remove block from available[k]
	available[k] = LINKF(block);
	
	if (available[k] != NULL)
		LINKB(available[k]) = &available[k];
	
	return block;
}
/*
 * Divides first available block with 2^k size into
 * 2 available blocks with 2^(k-1) sizes 
 * 
 */
void divide(uint8_t k)
{
	//Boundary check
	if (k >= MAX_CHUNK_SIZE + 1 || k < 1)
		return;
	
	void* block = pop_block(k);
	
	if (block == NULL)
		return;
	
	//DIVIDE BLOCK INTO 2 BLOCKS
	void* b1 = block;
	void* b2 = NEXT_BLOCK(block,k-1);
	
	KVAL(b1) = k-1;
	KVAL(b2) = k-1;
	
	TAG(b1) = 1;
	TAG(b2) = 1;
	
	//FORM LINKED LIST
	LINKB(b2) = b1;
	LINKF(b2) = available[k-1];
	
	LINKF(b1) = b2;
	LINKB(b1) = LINKB(&available[k-1]);
	
	if (available[k-1] != NULL)
	{
		LINKB(available[k-1]) = b2;
	}

	available[k-1] = b1;
}

int binit(void *chunkpointer, int chunksize)
{
	chunksize *= 1024;
	if (chunksize < (1 << MIN_CHUNK_SIZE) || chunksize > 1 << MAX_CHUNK_SIZE)
	    return -1;
	    
	uint8_t level = log2i((uint64_t)chunksize);//I DON'T KNOW WHY BUT CHUNK SIZE IS IN KB 
	
	//if (1 << level < chunksize)
	//	level = level + 1;
	
	available[level] = chunkpointer;
	TAG(chunkpointer) = 1;
	KVAL(chunkpointer) = level;
	LINKF(chunkpointer) = NULL;
	LINKB(chunkpointer) = &available[level];
	
	//Find pointers
	start_pointer = chunkpointer;
	end_pointer = start_pointer + (1 << level);
	
	return (0);		// if success
}

void *balloc(int objectsize)
{
	//BOUNDARY CHECK
	if (objectsize > (1 << MAX_REQUEST_SIZE) || objectsize < (1 << MIN_REQUEST_SIZE) )
		return NULL;
	
	//FIND BLOCK SIZE b
	uint8_t size = block_size(objectsize);
	
	//FIND MAX_CHUNK_SIZE > j >= size
	int j = size;
	
	while (j <= MAX_CHUNK_SIZE && available[j] == NULL)
		j++;
	
	//IF j > MAX_CHUNK_SIZE
		//RETURN NULL
	if (j > MAX_CHUNK_SIZE)
		return NULL;//No space found
	
	//Split
	while (j > size)
	{
		divide(j);
		j--;
	}
	
	//Get block
	void* block = pop_block(size);
	
	//Allocate block
	TAG(block) = 0;
	
	return DATA(block);
}

void* merge(void* block,void* bud)
{
    uint8_t k = KVAL(block);
    if (TAG(bud) != 0 && k < MAX_REQUEST_SIZE + 1)
    {
        //Remove bud from available list
        void* back = LINKB(bud);
        if (back == &available[k])
        {
            available[k] = LINKF(bud);
            
            if (available[k] != NULL)
                LINKB(available[k]) = &available[k];
        }
        else{
            LINKF(back) = LINKF(bud);
            
            if (LINKF(bud) != NULL)
                LINKB(LINKF(bud)) = LINKB(bud);
        }
        
        block = (block < bud) ? block : bud;
        
        KVAL(block) = k+1;
        
        return block;
    }
    return NULL;
}

void bfree(void *objectptr)
{
	if (objectptr == NULL || TAG(HEAD(objectptr)) != 0)
	    return;
	    
	void* block = HEAD(objectptr);
	void* bud = buddy(block);
	
	
	while (bud >= start_pointer && bud < end_pointer && TAG(bud) != 0 && KVAL(bud) < MAX_REQUEST_SIZE + 1)
	{
	    
	    block = merge(block,bud);
	    
	    bud = buddy(block);
	    
	}
	//INSERTION
	
	uint8_t k = KVAL(block);
	TAG(block) = 1;
	
	LINKB(block) = &available[k];
	LINKF(block) = available[k];
	
	if (available[k] != NULL)
	    LINKB(available[k]) = block;
	    
	available[k] = block;
	return;
}

void bprint(void)
{
	void* cur = start_pointer;
	
	while (cur < end_pointer)
	{
		uint8_t tag = TAG(cur);
		void* next = NEXT_BLOCK(cur,KVAL(cur));
		
		if (tag != 0)
			printf("%p-%p : free [TAG = %d , KVAL = %d , LINKF = %p , LINKB = %p]\n",cur,next-1,tag,KVAL(cur),LINKF(cur),LINKB(cur));
		else
			printf("%p-%p : allocated [TAG = %d , KVAL = %d]\n",cur,next-1,tag,KVAL(cur));
	
		cur = next;
	}
	return;
}
