#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "linked.c"

int main()
{
    char *str, *str2;

    /* Initial memory allocation */
    printf("Running malloc tests\n");
    str = (char *)malloc(13);
    str2 = (char *)malloc(13);
    strcpy(str, "mallocstring");
    assert(strcmp(str, "mallocstring") == 0);
    printf("Malloced string: %s\n", str);
    printf("Malloc tests PASSED \n");
    /* Reallocating memory */

    printf("Running realloc tests\n");
    str = (char *)realloc(str, 16);
    strcat(str, "new");
    assert(strcmp(str, "mallocstringnew") == 0);
    printf("Realloc tests PASSED \n");
    free(str);

    printf("Running calloc tests\n");
    int *a;

    a = (int *)calloc(3, sizeof(int));
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;

    assert(a[0] == 0);
    assert(a[1] == 1);
    assert(a[2] == 2);
    free(a);
    printf("Calloc tests PASSED \n");

    printf("All tests passed\n");
    return (0);
}