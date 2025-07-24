#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/user/syscall.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  int syscall_number = *(int*)f->esp;
  switch (syscall_number)
  {
    case SYS_HALT:{
      shutdown_power_off();
      NOT_REACHED();
      break;
    }
    case SYS_EXIT:{
      int status = *(int *)(f->esp + 4);
      struct thread *t = thread_current();
      t->exit_status = status;
      thread_exit();
      NOT_REACHED();
      break;
    }
    case SYS_EXEC:{
      char* file_name = *(char**)(f->esp + 4);
      f->eax = process_execute(file_name);
      break;
    }
    case SYS_WAIT:{
      pid_t pid = *(int *)(f->esp + 4);
      f->eax = process_wait(pid);
      // printf("f->eax : %d\n",f->eax);
      break;
    }
    case SYS_CREATE:{
      char* file_name = *(char**)(f->esp + 4);
      unsigned size = *(unsigned *)(f->esp + 8);
      f->eax = filesys_create(file_name, size);
      break;
    }
    case SYS_OPEN:{
      char* file_name = *(char**)(f->esp + 4);
      struct file* file = filesys_open(file_name);
      if (file == NULL){
        f->eax = -1;
      }
      else{
        f->eax = allocate_fd(file);
      }
      break;
    }
    case SYS_READ:{
      int fd = *(int*)(f->esp + 4);        // 첫 번째 인자
      void *buffer = *(void**)(f->esp + 8); // 두 번째 인자  
      unsigned size = *(unsigned*)(f->esp + 12); // 세 번째 인자

      if(fd == STDIN_FILENO){
        unsigned bytes_read = 0;
        char *buf = (char*)buffer;

        for(unsigned i = 0; i < size; i++){
          int c = input_getc();
          if(c == -1){
            break;
          }
          buf[i] = (char)c;
          bytes_read++;
        }
        f->eax = bytes_read;
      }else{
        struct file *file = get_file(fd);
        if(file == NULL){
          f->eax = -1;
        }else{
          f->eax = file_read(file,buffer,size);
        }
      }
      break;
    }
    case SYS_WRITE:{
      int fd = *(int*)(f->esp + 4);        // 첫 번째 인자
      void *buffer = *(void**)(f->esp + 8); // 두 번째 인자  
      unsigned size = *(unsigned*)(f->esp + 12); // 세 번째 인자

      if(fd == STDOUT_FILENO){
        putbuf(buffer,size);
        f->eax = size;
      }else{
        struct file *file = get_file(fd);
        if(file == NULL){
          f->eax = -1;
        }else{
          f->eax = file_write(file,buffer,size);
        }
      }
      break;
    }
    case SYS_SEEK:{
      int fd = *(int *)(f->esp+4);
      unsigned position = *(unsigned *)(f->esp+8);
      struct file* file = get_file(fd);
      if(file != NULL){
        file_seek(file,position);
      }
      break;
    }
    case SYS_CLOSE:{
      int fd = *(int *)(f->esp+4);
      file_close(get_file(fd));
      break;
    }
    case SYS_FILESIZE:{
      int fd = *(int *)(f->esp+4);
      struct file* file = get_file(fd);
      if(file == NULL){
        f->eax = -1;
      }else{
        f->eax = file_length(file);
      }
      break;
    }
    case SYS_TELL:{
      int fd = *(int *)(f->esp+4);
      struct file* file = get_file(fd);
      if (file == NULL){
        f->eax = -1;
      }
      else{
        f->eax = file_tell(file);
      }
      break;
    }
    case SYS_REMOVE:{
      char* file_name = *(char**)(f->esp + 4);
      f->eax = filesys_remove(file_name);
      break;
    }
  }
  
  // printf ("system call!\n");
  // thread_exit ();
}
