#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  switch (*((uint32_t*)f->esp)){
    case SYS_CREATE:
    case SYS_OPEN:
      char* file_name = *(char**)(f->esp + 4);
      struct file* file = filesys_open(file_name);
      if (file == NULL){
        f->eax = -1;
      }
      else{
        f->eax = allocate_fd(file);
      }
      break;
    case SYS_READ:
    case SYS_WRITE:
    case SYS_SEEK:
    case SYS_CLOSE:
      int fd = *(int *)(f->esp+4);
      file_close(get_file(fd));
      break;
    case SYS_FILESIZE:
      
    case SYS_TELL:
      int fd = *(int *)(f->esp+4);
      struct file* file = get_file(fd);
      if (file == NULL){
        f->eax = -1;
      }
      else{
        f->eax = file->pos;
      }
      break;
    case SYS_REMOVE:
  }
  // printf ("system call!\n");
  // thread_exit ();
}
