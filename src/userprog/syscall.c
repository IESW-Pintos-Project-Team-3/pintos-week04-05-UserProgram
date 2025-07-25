#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "lib/user/syscall.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

static inline int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
};

/*일단 여기다 만듦 나중에 필요하면 다른 곳에 선언, 정의
  Check the address is valid */
static bool validate_esp(struct intr_frame *f, int count, int k){
  for(int i = 0; i < count; i++){
      if(!is_user_vaddr(f->esp + i*4 + k) || get_user(f->esp + i*4 + k) == -1){
        return false;
      }
      if(!is_user_vaddr(f->esp + i*4 + k + 1) || get_user(f->esp + i*4 + k + 1) == -1){
        return false;
      }
      if(!is_user_vaddr(f->esp + i*4 + k + 2) || get_user(f->esp + i*4 + k + 2) == -1){
        return false;
      }
      if(!is_user_vaddr(f->esp + i*4 + k + 3) || get_user(f->esp + i*4 + k + 3) == -1){
        return false;
      }
  }
  return true;
}

static inline bool validate_string(char* str){
  for(int i = 0; i < READDIR_MAX_LEN;i++){
    if(!is_user_vaddr(str + i) || get_user(str + i) == -1){
        return false;
    }

    if(str[i] == '\0'){
      return true;
    }
  }

  return false;
}

static inline bool validate_buffer(void* buf, unsigned size){
  for(int i = 0; i < size; i++){
    if(!is_user_vaddr(buf + i) || get_user(buf + i) == -1){
        return false;
    }
  }

  return true;
}

static inline void __exit(int status){
  struct thread *t = thread_current();
  t->exit_status = status;
  thread_exit();
  NOT_REACHED();
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  // if (!is_user_vaddr(f->esp) || pagedir_get_page(thread_current()->pagedir, pg_round_down(f->esp)) == NULL){
  //   __exit(-1);
  //   return;
  // }
  validate_esp(f, 1, 0);

  int syscall_number = *(int*)f->esp;
  switch (syscall_number)
  {
    case SYS_HALT:{
      shutdown_power_off();
      NOT_REACHED();
      break;
    }
    case SYS_EXIT:{
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

      int status = *(int *)(f->esp + 4);
      __exit(status);
      break;
    }
    case SYS_EXEC:{
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

      char *file_name = *(char**)(f->esp + 4);
      if (!validate_string(file_name)){
        __exit(-1);
      }

      f->eax = process_execute(file_name);
      break;
    }
    case SYS_WAIT:{
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

      pid_t pid = *(int *)(f->esp + 4);
      f->eax = process_wait(pid);
      // printf("f->eax : %d\n",f->eax);
      break;
    }
    case SYS_CREATE:{
      if (!validate_esp(f, 2, 4)){
        __exit(-1);
      }

      char *file_name = *(char**)(f->esp + 4);
      if (!validate_string(file_name)){
        __exit(-1);
      }

      unsigned size = *(unsigned *)(f->esp + 8);
      f->eax = filesys_create(file_name, size);
      break;
    }
    case SYS_OPEN:{
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

      char *file_name = *(char**)(f->esp + 4);
      if (!validate_string(file_name)){
        __exit(-1);
      }

      struct file *file = filesys_open(file_name);
      if (file == NULL){
        f->eax = -1;
      }
      else{
         f->eax = allocate_fd(file);
      }
      break;
    }
    case SYS_READ:{
      if (!validate_esp(f, 3, 4)){
        __exit(-1);
      }

      int fd = *(int*)(f->esp + 4);        // 첫 번째 인자
      void *buffer = *(void**)(f->esp + 8); // 두 번째 인자
      unsigned size = *(unsigned*)(f->esp + 12); // 세 번째 인자
      if(!validate_buffer(buffer, size)){
        __exit(-1);
      }

      if (fd < 0 || fd >= 128){
        f->eax = -1;
        break;
      }

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
      if (!validate_esp(f, 3, 4)){
        __exit(-1);
      }

      int fd = *(int*)(f->esp + 4);        // 첫 번째 인자
      void *buffer = *(void**)(f->esp + 8); // 두 번째 인자  
      unsigned size = *(unsigned*)(f->esp + 12); // 세 번째 인자
      if(!validate_buffer(buffer, size)){
        __exit(-1);
      }
      
      if (fd < 0 || fd >= 128){
        f->eax = -1;
        break;
      }

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
      if (!validate_esp(f, 2, 4)){
        __exit(-1);
      }
      
      int fd = *(int *)(f->esp+4);
      unsigned position = *(unsigned *)(f->esp+8);
      struct file *file = get_file(fd);
      if(file != NULL){
        file_seek(file,position);
      }
      break;
    }
    case SYS_CLOSE:{
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

      int fd = *(int *)(f->esp+4);
      struct file *file = get_file(fd);
      if (file != NULL){
        file_close(file);
        thread_current()->fd_table[fd] = NULL;
      }
      break;
    }
    case SYS_FILESIZE:{
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

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
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

      int fd = *(int *)(f->esp+4);
      struct file *file = get_file(fd);
      if (file == NULL){
        f->eax = -1;
      }
      else{
        f->eax = file_tell(file);
      }
      break;
    }
    case SYS_REMOVE:{
      if (!validate_esp(f, 1, 4)){
        __exit(-1);
      }

      char *file_name = *(char**)(f->esp + 4);
      if (!validate_string(file_name)){
        __exit(-1);
      }
      
      f->eax = filesys_remove(file_name);
      break;
    }
  }
  
  // printf ("system call!\n");
  // thread_exit ();
}
