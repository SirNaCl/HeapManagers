#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>

#define MINEXP 5 // Smallest possible block = 2^MINEXP
#define LEVELS 22
#define MAGIC 123456789
#define BLOCKSIZE 1 << (LEVELS + MINEXP - 1) // Largest possible block = 2^(MINEXP+LEVELS-1)

typedef struct head_t head_t;

struct head_t
{
    int used;  // 0 = free
    int level; // 0 smallest possible block, LEVELS = largest possible block
    int magic;
    head_t *next;
};
#define HEAD_SIZE sizeof(head_t)

head_t *root = NULL;
// free blocks at a given level (sublists works as stacks)
head_t *free_blocks[LEVELS - 1] = {NULL};

void insert_free(head_t *b)
{
    assert(!b->used);
    b->next = free_blocks[b->level];
    free_blocks[b->level] = b;
}

void remove_free(head_t *b)
{
    head_t *prev = NULL;
    head_t *next = free_blocks[b->level];

    while (next && next != b)
    {
        prev = next;
        next = next->next;
    }

    // next == b
    if (next)
    {
        // not first iteration
        if (prev)
            prev->next = next->next;
        else
            free_blocks[b->level] = next->next;
    }

    // Never found b, throw error?
}

// Generate a new block that can be used as root
head_t *new_block()
{
    long int mask = 0xfffff << LEVELS + MINEXP;
    long int current = (long int)sbrk(0);
    long int trash = (long int)sbrk(~(current | mask) + 1); // Move break to next aligned address
    long int adr = (long int)sbrk(BLOCKSIZE << 1);          // Allocate largest block plus alignment buffer

    head_t *n = (head_t *)adr;

    // Allocation failed
    if (!n)
        return NULL;

    // Make sure address i properly aligned (enough zeroes at end of address)
    assert(((long int)n & (BLOCKSIZE - 1)) == 0);
    n->level = LEVELS - 1; // Set size of block to largest possible
    n->magic = MAGIC;
    n->used = 0;
    insert_free(n);
    return n;
}

// Returns the address to the given block's buddy
head_t *get_buddy(head_t *block)
{
    assert(block->magic = MAGIC);
    int index = block->level;
    // Shift the one to wanted position,
    // the mask adds / subtracts the memory address by the block's size by toggling the given bit
    long int adr_mask = 0x1 << (index + MINEXP); // Create mask that can be used to toggle bit of right lvl
    return (struct head_t *)((long int)block ^ adr_mask);
}

// split the given block and return the address to the new buddy
head_t *split(head_t *block)
{
    assert(block->level > 0);

    // move the block between free levels
    remove_free(block);
    block->level -= 1;
    insert_free(block);

    // TODO: use get_buddy
    int index = block->level;
    long int mask = 0x1 << (index + MINEXP);
    head_t *b = (head_t *)((long int)block | mask);

    // head_t *b = get_buddy(block);
    b->level = block->level;
    b->magic = MAGIC;
    b->used = 0;

    return b;
}

head_t *merge(head_t *block)
{
    long int mask = 0xffffffffffffff << (1 + block->level + MINEXP);
    // The block of the buddy pair with lower memory (primary) can expand over the other one
    head_t *primary = (head_t *)((long int)block & mask);
    head_t *b = get_buddy(primary);
    b->magic = 0; // Invalidate buddy
    remove_free(b);
    remove_free(primary);
    primary->level++;
    insert_free(primary);
    return primary;
}

void *data_adr(head_t *h)
{
    return (void *)(h + 1);
}

head_t *get_head(void *adr)
{
    return ((head_t *)adr - 1);
}

int req_lvl(int size)
{
    int tot = size + HEAD_SIZE;

    int lvl = 0;
    int s = 1 << MINEXP;

    while (tot > s)
    {
        s <<= 1;
        lvl += 1;
    }

    assert(lvl < LEVELS);
    return lvl;
}

head_t *find_free(int level)
{
    head_t *b;
    if ((b = free_blocks[level]))
        free_blocks[level] = b->next;

    return b;
}

head_t *get_block(int level)
{
    if (!root)
        root = new_block();

    short split_count = 0;
    head_t *block = find_free(level);

    // find block of correct size or a splittable block
    while (!block && split_count <= LEVELS - level)
    {
        split_count += 1;
        block = find_free(level + split_count);
    }

    // Split block into correct level if needed
    while (block && split_count-- > 0)
        block = split(block);

    if (block)
        block->used = 1;

    return block;
}

int should_merge(head_t *block)
{
    if (block->level >= LEVELS)
        return 0;

    head_t *b = get_buddy(block);

    return b->level == block->level && !b->used;
}

void unassign(head_t *block)
{
    block->used = 0;

    while (should_merge(block))
        block = merge(block);

    return;
}

void *malloc(size_t size)
{
    if (size <= 0)
        return NULL;

    int index = req_lvl(size);
    head_t *block = get_block(index);

    if (!block)
        return NULL;

    return data_adr(block);
}

void free(void *adr)
{
    if (adr != NULL)
    {
        head_t *block = get_head(adr);
        unassign(block);
    }
    return;
}

void *realloc(void *ptr, size_t size)
{
    // Return new block if NULL pointer
    if (!ptr)
        return malloc(size);

    // Get the head for the block at ptr
    head_t *block = get_head(ptr);
    assert(block->used); // The block should not be free

    int level = req_lvl(size);

    // No changes if size is the same
    if (level <= block->level)
        return ptr;

    // Find new block if increasing size
    // Allocate new block
    void *nb = malloc(size);
    if (!nb)
        return NULL;

    // Move content to new block
    memcpy(nb, ptr, (1 << (block->level + MINEXP)) - HEAD_SIZE);
    free(ptr);
    return nb;
}

void *calloc(size_t nbr, size_t size)
{
    size_t tot_size = nbr * size;
    void *ptr = malloc(tot_size);
    // Return NULL if allocation failed
    if (!ptr)
        return NULL;

    // Set data to 0
    memset(ptr, 0, tot_size);
    return ptr;
}
