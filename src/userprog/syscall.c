#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  // intr_frame의 모든 내용을 출력
  intr_dump_frame(f);
  
  // 시스템 콜 번호도 직접 출력
  printf("System call number: %d\n", *(int*)f->esp);
  
  // 스택 내용을 더 자세히 출력
  printf("Stack contents:\n");
  printf("  f->esp + 0:  %d (syscall number)\n", *(int*)f->esp);
  printf("  f->esp + 4:  %p (arg1)\n", *(void**)(f->esp + 4));
  printf("  f->esp + 8:  %p (arg2)\n", *(void**)(f->esp + 8));
  printf("  f->esp + 12: %p (arg3)\n", *(void**)(f->esp + 12));
  
  printf ("system call!\n");
  thread_exit ();
}
