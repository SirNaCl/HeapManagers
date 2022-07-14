#include <stdlib.h>
#include <stdio.h>

void *malloc(size_t size)
{
    write(0, "Malloc", sizeof "Malloc");
    return NULL;
}
