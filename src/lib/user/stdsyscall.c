#include <stdsyscall.h>

struct FILE{
    void* rd_buffer;
    void* wr_buufer;
    int rd_pos;
    int wr_pos;
    int old_pos;
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

struct FILE *
fdopen(int fd, int type)
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
  f->fd = fd;
  return f;
}

void
fclose(struct FILE* f){
  if (f == NULL){
    return;
  }
  
//   if (f->wr_pos){
//     // /*If file position is different from buffer's write position
//     //   it set fileposition to old position*/
//     // if (tell(f->fd) != f->old_pos){
//     //   seek(f->fd, f->old_pos);
//     // }


//   }
  fflush(f);

  close(f->fd);

  free(f->rd_buffer);
  free(f->wr_buufer);
  free(f);
}

int
fread(void* p, size_t a, size_t b, struct FILE* f)
{
  if (f == NULL || f->type == WRONLY){
    return -1;
  }

  size_t size = a * b;
  if (size < a || size < b){
    return -1;
  }
  
  int read_bytes = 0;
//   if (f->old_pos != tell(f->fd)){
//     f->rd_pos = 0;
//   }

  if (size <= f->rd_pos){
    for (int i = 0; i < size; i++){
        p[i] = f->rd_buffer[i];
        read_bytes++;
    }
    memmove(f->rd_buffer, f->rd_buffer + size, f->rd_pos - size);
    f->rd_pos -= size;
  }else{
    for (int i = 0; i < f->rd_pos; i++){
        p[read_bytes++] = f->rd_buffer[i];
    }
    size -= f->rd_pos;
    read_bytes += read(f->fd, p + read_bytes, size);
    f->rd_pos = 0;
    // while(size){
    //   if (!read(f->fd, f->rd_buffer, f->buf_size)){
    //     break;
    //   }
    //   if (size >= f->buf_size){
    //     for(int i = 0; i < f->buf_size; i++){
    //       p[read_bytes++] = f->rd_buffer[i];
    //     }
    //     size -= f->buf_size;
    //   }else{
    //     for(int i = 0; i < size; i++){
    //       p[read_bytes++] = f->rd_buffer[i];
    //     }
    //     f->rd_pos = f->buf_size - size;
    //     memmove(f->rd_buffer, f->rd_buffer + size, f->rd_pos);
    //     size = 0;
    //   }
    // }
  }

//   if (read_bytes){
//     f->old_pos = tell(f->fd);
//   }

  return read_bytes;
}

int
fwrite(void* p, size_t a, size_t b, struct FILE* f)
{
  if (f == NULL || f->type == RDONLY){
    return -1;
  }

  size_t size = a * b;
  if (size < a || size < b){
    return -1;
  }

  int write_bytes = 0;
  if (size <= f->buf_size - f->wr_pos){
    for(int i = 0; i < size; i++){
      f->wr_buufer[f->wr_pos++] = p[write_bytes++];
    }
    if (f->buf_size == f->wr_pos){
      fflush(f);
    }
  }else{
    fflush(f);
    write_bytes = write(f->fd, p, size);
  }

  return write_bytes;
}

void
fseek(unsigned position, struct FILE *f)
{
  if (f == NULL){
    return;
  }

  fflush(f);
  f->rd_pos;
  
  seek(f->fd, position);
}

void
fflush(struct FILE *f)
{
  if (f == NULL){
    return;
  }

  if(f->wr_pos){
    write(f->fd, f->wr_buufer, f->wr_pos);
    f->wr_pos = 0;
  }
}