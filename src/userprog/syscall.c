#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
<<<<<<< HEAD

// ====================
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "process.h"
#include "threads/palloc.h"
#define MAXCALL 20

static void syscall_handler (struct intr_frame *);

// =========================
void ExitStatus(int status);
void IWrite(struct intr_frame * f);
void IExit(struct intr_frame * f);
void IHALT(struct intr_frame * f);
void IEXEC(struct intr_frame * f);
void IWAIT(struct intr_frame * f);
typedef void(*CALL_PROC)(struct  intr_frame*);
CALL_PROC pfn[MAXCALL];
=======
static void syscall_handler (struct intr_frame *);

//======

CALL_PROC call_arr[MAXCALL];
void system_write(struct intr_frame* f);
>>>>>>> 8ed703ee41d6b6d85d6f88f57daf30d20a717601


bool is_valid_addr(const void *vaddr)
{
	void *page_ptr = pagedir_get_page(thread_current()->pagedir, vaddr);

	if (!is_user_vaddr(vaddr) || page_ptr == NULL)
	{
    return false;
	}
	return true;
}


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  //init call_arr with null
  for(int i=0;i<MAXCALL;i++){
    call_arr[i]=NULL;
  }
<<<<<<< HEAD
  pfn[SYS_WRITE] = IWrite;
  pfn[SYS_EXIT] = IExit;
  pfn[SYS_HALT] = IHALT;
  pfn[SYS_EXEC] = IEXEC;
  pfn[SYS_WAIT] = IWAIT;
=======
  call_arr[SYS_WRITE] = system_write;
>>>>>>> 8ed703ee41d6b6d85d6f88f57daf30d20a717601
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
<<<<<<< HEAD
  if( !is_valid_addr(f->esp) ){
    ExitStatus(-1);
  }
  int No = *(int*)(f->esp);
  if(No>MAXCALL || No < 0)
  {
    // printf("We don't have this system call!\n");
    ExitStatus(-1);
  }
  if(pfn[No] == NULL)
  {
    printf("have not implemenyt this system call\n");
    ExitStatus(-1);
=======
  int No = *(int*)(f->esp);
  if(call_arr[No] != NULL){
    call_arr[No](f);
>>>>>>> 8ed703ee41d6b6d85d6f88f57daf30d20a717601
  }
  printf ("system call!\n");
  thread_exit ();
}

<<<<<<< HEAD
void ExitStatus(int status){
  struct  thread* cur = thread_current();
  cur -> rtv = status;
  thread_exit();
}

void IExit(struct intr_frame * f)
{
  int *esp = (int*)f->esp;
  if(!is_user_vaddr(esp+1))
    ExitStatus(-1);
  else{
    int rtv = *(esp+1);
    ExitStatus(rtv);
  }
}

void IWrite(struct intr_frame * f)
{
  int *esp = (int*)f->esp;
=======
void 
system_write(struct intr_frame* f){
   int *esp = (int*)f->esp;
>>>>>>> 8ed703ee41d6b6d85d6f88f57daf30d20a717601
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
<<<<<<< HEAD

}

void IHALT(struct intr_frame * f)
{
  shutdown_power_off();
  f->eax = 0;
}


void IEXEC(struct intr_frame * f){

  int *esp = (int*)f->esp;
  if(!is_user_vaddr(esp+1))
    ExitStatus(-1);
  else{
    char* file = *(esp+1);
    f->eax = process_execute(file);
  }
}
void IWAIT(struct intr_frame * f){
  int *esp = (int*)f->esp;
  // if(!is_user_vaddr(esp+7))
    // ExitStatus(-1);
  tid_t wait_id = *(esp+1);
  process_wait(wait_id);

=======
>>>>>>> 8ed703ee41d6b6d85d6f88f57daf30d20a717601
}