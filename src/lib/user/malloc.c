#include <malloc.h>
#include <string.h>
#include "../syscall-nr.h"

/* Invokes syscall NUMBER, passing argument ARG0, and returns the
   return value as an `int'. */
#define syscall1(NUMBER, ARG0)                                           \
        ({                                                               \
          int retval;                                                    \
          asm volatile                                                   \
            ("pushl %[arg0]; pushl %[number]; int $0x30; addl $8, %%esp" \
               : "=a" (retval)                                           \
               : [number] "i" (NUMBER),                                  \
                 [arg0] "g" (ARG0)                                       \
               : "memory");                                              \
          retval;                                                        \
        })

void *
malloc(size_t size)
{
    return (void *) syscall1 (SYS_MALLOC, size);
}

void *
calloc(size_t a, size_t b)
{
  void *p;
  size_t size;

  /* Calculate block size and make sure it fits in size_t. */
  size = a * b;
  if (size < a || size < b)
    return NULL;

  /* Allocate and zero memory. */
  p = (void *) syscall1 (SYS_MALLOC, size);
  if (p != NULL)
    memset (p, 0, size);

  return p;
}

void *
realloc(void * old, size_t size)
{

}

void free(void *p)
{
  syscall1 (SYS_FREE, p);
}