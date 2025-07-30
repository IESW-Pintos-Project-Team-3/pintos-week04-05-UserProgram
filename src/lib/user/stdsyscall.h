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
struct FILE *fdopen(int, int);
void fclose(struct FILE *);
int fread(void *, size_t, size_t, struct FILE *);
int fwrite(void *, size_t, size_t, struct FILE *);
void fseek(unsigned, struct FILE *);
void fflush(struct FILE *);
