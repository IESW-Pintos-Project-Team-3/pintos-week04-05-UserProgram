#ifndef LIB_USER_MALLOC
#define LIB_USER_MALLOC

#include <stddef.h>

void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);

#endif