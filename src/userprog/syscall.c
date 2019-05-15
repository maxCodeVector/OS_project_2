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
  //init call_arr with null
  for(int i=0;i<MAXCALL;i++){
    call_arr[i]=NULL;
  }
  call_arr[SYS_WRITE] = system_write;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}

void 
system_write(struct intr_frame* f){
   int *esp = (int*)f->esp;
  // if(!is_user_vaddr(esp+7))
    // ExitStatus(-1);
  int fd = *(esp+1);
  char * buffer = (char*)*(esp+2);
  unsigned int size = *(esp+3);

  if(fd==1){
    putbuf(buffer, size);
    f->eax = 0;
  }else{
    printf("I only can print in console!\n");
  }
}