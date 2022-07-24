#include <stdio.h>
#include <assert.h>
#include <string.h>
//#include <stdlib.h>

int main()
{
    char *str;

    /* Initial memory allocation */
    str = (char *)malloc(12);
    strcpy(str, "mallocstring");
    assert(strcmp(str, "mallocstring") == 0);

    /* Reallocating memory */
    str = (char *)realloc(str, 15);
    strcat(str, "new");
    assert(strcmp(str, "mallocstringnew") == 0);

    free(str);
    printf("Tests passed\n");
    return (0);
}