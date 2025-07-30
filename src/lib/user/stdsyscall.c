#include <stdsyscall.h>

struct FILE{
    void* rd_buffer;
    void* wr_buufer;
    int rd_pos;
    int wr_pos;
    int buf_size;
    int fd;
    int type;
};

struct FILE *
fopen(const char *file_name, int type)
{
  struct FILE* f = calloc(1, sizeof *f);
  if (f == NULL){
    return NULL;
  }

  f->rd_buffer = calloc(1, 512);
  f->wr_buufer = calloc(1, 512);
  if (f->rd_buffer == NULL || f->wr_buufer == NULL){
    free(f->rd_buffer);
    free(f->wr_buufer);
    free(f);
    return NULL;
  }
  f->buf_size = 512;
  f->type = type;
  f->fd = open(file_name);
  return f;
}

void fclose(struct File*);
int fread(void*, size_t, size_t, struct File*);
int fwrite(void*, size_t, size_t, struct File*);