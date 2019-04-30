#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#define MAXCALL 20

static void syscall_handler (struct intr_frame *);

// =========================
void ExitStatus(int status);
void IWrite(struct intr_frame * f);
void IExit(struct intr_frame * f);
typedef void(*CALL_PROC)(struct  intr_frame*);
CALL_PROC pfn[MAXCALL];

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  int i;
  for(i=0;i<MAXCALL;i++){
    pfn[i] = NULL;
  }
  pfn[SYS_WRITE] = IWrite;
  pfn[SYS_EXIT] = IExit;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // if(!is_user_vaddr(f->esp)){
  //   ExitStatus(-1);
  // }
  int No = *(int*)(f->esp);
  if(No>MAXCALL || No < 0)
  {
    printf("We don't have this system call!\n");
    ExitStatus(-1);
  }
  if(pfn[No] == NULL)
  {
    printf("have not implemenyt this system call\n");
    ExitStatus(-1);
  }
  pfn[No](f);
  // printf ("system call!\n");
  // thread_exit ();
}

void ExitStatus(int status){
  struct  thread* cur = thread_current();
  cur -> rtv = status;
  thread_exit();
}

void IExit(struct intr_frame * f)
{
  thread_exit();
}

void IWrite(struct intr_frame * f)
{
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
