#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void *malloc(size_t size)
{
    write(STDOUT_FILENO, "Malloc", sizeof "Malloc");
    return NULL;
}
