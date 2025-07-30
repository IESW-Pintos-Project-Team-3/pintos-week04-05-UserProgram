#include <syscall.h>
#include <stddef.h>
struct FILE;

enum
{
  RDONLY,
  WRONLY,
  RDWR
};

struct FILE *fopen(const char *, int);
void fclose(struct File*);
int fread(void*, size_t, size_t, struct File*);
int fwrite(void*, size_t, size_t, struct File*);
