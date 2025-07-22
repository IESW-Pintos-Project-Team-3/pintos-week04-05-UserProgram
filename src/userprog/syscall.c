#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/user/syscall.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int syscall_number = *(int*)f->esp;
  switch (syscall_number)
  {
    case SYS_WRITE:
      int fd = *(int*)(f->esp + 4);        // 첫 번째 인자
      void *buffer = *(void**)(f->esp + 8); // 두 번째 인자  
      unsigned size = *(unsigned*)(f->esp + 12); // 세 번째 인자

      if(fd == STDOUT_FILENO){
        putbuf(buffer,size);
        f->eax = size;
      }else{
        f->eax = -1;
      }
      break;
  }
  printf ("system call!\n");
  thread_exit ();
}
