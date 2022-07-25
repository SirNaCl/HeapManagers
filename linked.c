#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define MINSIZE 8

// Normalize the data size to a minimum allocation size
#define NORMALIZE(size) (size + (MINSIZE - size % MINSIZE))
typedef struct block_head_t block_head_t;

struct block_head_t
{
    int free : 1;
    size_t size;
    block_head_t *next;
};

#define HEADSIZE (NORMALIZE(sizeof(block_head_t)))

block_head_t *root = NULL;

// Increase the heap with given size and return block header
// last_free = last free head in linked list
block_head_t *grow_heap(block_head_t *last_free, size_t size)
{
    block_head_t *new_head = sbrk(0); /*Assign new head's address att current heap break*/
    void *block_end = sbrk(NORMALIZE(size + HEADSIZE));

    // return NULL if allocation failed
    if (block_end == (void *)0)
    {
        return NULL;
    }

    new_head->size = size;
    new_head->free = 0;
    new_head->next = NULL;

    // Assign new break as next free address
    if (last_free)
    {
        last_free->next = block_end;
    }

    return new_head;
}

block_head_t *claim_and_split(block_head_t *block, size_t size)
{
    block->free = 0;
    if (block->size - HEADSIZE - size >= MINSIZE)
    {
        // Split excess into new block if worthwhile
        block_head_t *nb = block + NORMALIZE(size);
        nb->next = block->next;
        nb->free = 1;
        nb->size = NORMALIZE(block->size - HEADSIZE - size);
        block->next = nb;
        block->size = size;
    }

    return block;
}

block_head_t *find_free(size_t size)
{
    block_head_t *block = root;
    while (block)
    {
        if (block->free && size <= block->size)
        {
            return claim_and_split(block, size);
        }

        if (!block->next)
            break;

        block = block->next;
    }

    return grow_heap(block, size);
}

void consolidate_blocks()
{
    block_head_t *block = root;
    while (block && block->next)
    {
        if (block->free && block->next->free)
        {
            block->size += block->next->size + HEADSIZE;
            block->next = block->next->next;
            continue; // If we consolidated, we don't have to jump to next block
        }

        block = block->next;
    }
}
// Allocate a block of size "size" to the heap
void *malloc(size_t size)
{
    if (size <= 0)
        return NULL;

    if (!root)
    {
        block_head_t *block = grow_heap(NULL, size);
        if (!block)
            return NULL;

        root = block;
        return (block + 1);
    }

    block_head_t *f = find_free(size);
    if (!f)
        return NULL;

    return (f + 1);
}

void free(void *ptr)
{
    if (!ptr)
        return;
    block_head_t *block = (block_head_t *)ptr - 1;
    assert(block->free == 0);
    block->free = 1;
    consolidate_blocks();
}

void *realloc(void *ptr, size_t size)
{
    // Return null if invalid pionter or size
    if (!ptr)
        return malloc(size);

    block_head_t *block = ((block_head_t *)ptr) - 1;
    assert(block->free == 0);

    // No changes if size is the same
    if (NORMALIZE(size) == block->size)
        return ptr;

    // Reduce size
    if (size < block->size)
        return (claim_and_split(block, size) + 1);

    // Find new block if increasing size
    // TODO: Check if block can expand
    void *nb = malloc(size);
    if (!nb)
        return NULL;

    // Move content to new block
    memcpy(nb, ptr, block->size);
    free(ptr);
    return nb;
}

void *calloc(size_t nbr, size_t size)
{
    size_t tot_size = nbr * size;
    void *ptr = malloc(tot_size);
    if (ptr == NULL)
        return NULL;
    memset(ptr, 0, tot_size);
    return ptr;
}