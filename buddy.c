#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>

#define MINEXP 4 // Smallest possible block = 2^MINEXP
#define LEVELS 8 // Largest possible block = 2^(MINEXP+LEVELS-1) = 2^12 = 4ki
#define MAGIC 123456789
#define BLOCKSIZE (1 << (LEVELS + MINEXP - 1)) // 2^(MINEXP+LEVEL)
#define ALIGNMENT 12
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct head_t head_t;

struct head_t
{
    int used;        // 0 = free
    short int level; // 0 smallest possible block, LEVELS (8) = largest possible block
    int magic;
};
#define HEAD_SIZE (ALIGN(sizeof(head_t)))

head_t *root = NULL;

// Generate a new block that can be used as root
head_t *new_block()
{
    head_t *n = (head_t *)sbrk(BLOCKSIZE);

    if (n == MAP_FAILED)
        return NULL;

    assert(((long int)n & 0xfff) == 0);
    n->level = LEVELS - 1; // Set size of block to largest possible
    n->magic = MAGIC;
    return n;
}

// Returns the address to the given block's buddy
head_t *get_buddy(head_t *block)
{
    int index = block->level;
    // Shift the one to wanted position,
    // the mask adds / subtracts the memory address by the block's size by toggling the given bit
    long int adr_mask = 0x1 << (index + MINEXP);
    return (struct head_t *)((long int)block ^ adr_mask);
}

// split the given block and return the address to the new buddy
head_t *split(head_t *block)
{
    int index = block->level - 1;
    block->level--;
    int mask = 0x1 << (block->level + MINEXP);
    head_t *b = ((long int)block | mask);
    b->level = block->level;
    b->magic = MAGIC;
    return b;
}

head_t *merge(head_t *block)
{
    long int mask = 0xffffffffffffff << (1 + block->level + MINEXP);
    head_t *primary = (head_t *)((long int)block & mask);
    primary->level++;
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
    int tot = ALIGN(size + HEAD_SIZE);

    int lvl = 0;
    int s = 1 << MINEXP;

    while (tot > s)
    {
        s <<= 1;
        lvl += 1;
    }

    return lvl;
}

head_t *find_free(int level)
{
    long int mask = 0;
    head_t *block = root;
    while (mask < BLOCKSIZE)
    {
        // Check if there is a valid block at address
        if (block->magic == MAGIC && !block->used && block->level == level)
            return block;

        block = (head_t *)((long int)root ^ mask);
        mask += 0x1 << (MINEXP + level);
    }
    return NULL;
}

head_t *get_block(int level)
{
    if (!root)
        root = new_block();

    short split_count = 0;
    head_t *block = find_free(level + split_count);

    // find block of correct size or a splittable block
    while (!block && split_count <= LEVELS - level)
    {
        split_count += 1;
        block = find_free(level + split_count);
    }

    while (block && split_count-- > 0)
        block = split(block);

    if (block)
        block->used = 1;

    return block;
}

int should_merge(head_t *block)
{
    return !block->used && !get_buddy(block)->used;
}

void unassign(head_t *block)
{
    block->used = 0;

    if (should_merge(block))
        merge(block);

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
    memcpy(nb, ptr, 1 << (block->level + MINEXP));
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
