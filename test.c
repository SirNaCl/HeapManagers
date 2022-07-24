#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "linked.c"

int main()
{
    char *str;

    /* Initial memory allocation */
    printf("Running malloc tests");
    str = (char *)malloc(13);
    strcpy(str, "mallocstring");
    assert(strcmp(str, "mallocstring") == 0);
    printf("Malloc tests PASSED \n");
    /* Reallocating memory */

    printf("Running realloc tests");
    str = (char *)realloc(str, 16);
    strcat(str, "new");
    assert(strcmp(str, "mallocstringnew") == 0);
    printf("Realloc tests PASSED \n");

    free(str);
    printf("All tests passed\n");
    return (0);
}