#include <stdlib.h>
#include <stdio.h>

void *malloc(size_t size)
{
    printf('Our malloc');
    return NULL;
}
