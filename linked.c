#include <stdlib.h>
#include <unistd.h>

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

// Increase the heap with given size and return block header
// last_free = last free head in linked list
block_head_t *grow_heap(block_head_t *last_free, size_t size)
{
    block_head_t *new_head = sbrk(0); /*Assign new head's address att current heap break*/
    void *block_end = sbrk(NORMALIZE(size + HEADSIZE));

    // return NULL if allocation failed
    if (block_end != (void *)0)
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

block_head_t find_free() {}

// Allocate a block of size "size" to the heap
void *malloc(size_t size)
{
    if (size <= 0)
    {
        return NULL;
    }
}
