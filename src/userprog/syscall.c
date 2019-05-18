#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

// =========== nessary include =========
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "process.h"
#include "threads/palloc.h"
#include "filesys/off_t.h"
#define MAXCALL 20

static void syscall_handler(struct intr_frame *);

// =============all syscall project 2 need to implement ============

int syscall_HALT(struct intr_frame *f);     /* Halt the operating system. */
int syscall_EXIT(struct intr_frame *f);     /* Terminate this process. */
int syscall_EXEC(struct intr_frame *f);     /* Start another process. */
int syscall_WAIT(struct intr_frame *f);     /* Wait for a child process to die. */
int syscall_CREATE(struct intr_frame *f);   /* Create a file. */
int syscall_REMOVE(struct intr_frame *f);   /* Delete a file. */
int syscall_OPEN(struct intr_frame *f);     /* Open a file. */
int syscall_FILESIZE(struct intr_frame *f); /* Obtain a file's size. */
int syscall_READ(struct intr_frame *f);     /* Read from a file. */
int syscall_WRITE(struct intr_frame *f);    /* Write to a file. */
int syscall_SEEK(struct intr_frame *f);     /* Change position in a file. */
int syscall_TELL(struct intr_frame *f);     /* Report current position in a file. */
int syscall_CLOSE(struct intr_frame *f);    /* Close a file. */
struct process_file* search_fd(struct list* files, int fd);

typedef int (*syscall_hander_function)(struct intr_frame *);
void process_exit_with_status(int status); // this function make current thread exit itself with exit code: status
syscall_hander_function pfn[MAXCALL];

// this funcion check address if valid, it need to implement more completely later
bool is_valid_addr(const void *vaddr)
{
  // void *page_ptr = 

  if (!is_user_vaddr(vaddr) || pagedir_get_page(thread_current()->pagedir, vaddr) == NULL)
  {
    return false;
  }
  return true;
}

// get the arguments in esp and put it in address arg, also each time invoke it need to check if address is valid
void pop_stack(int* esp, int* arg, int offset){
  if( is_valid_addr(esp+offset) ){
    *arg = *(esp+offset);
  }else
  {
    process_exit_with_status(-1);
  }
}



void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  int i;
  for (i = 0; i < MAXCALL; i++)
  {
    pfn[i] = NULL;
  }
  /** 
 * ========those lines register behaviour hander for each syscall==========
 * just need to write the concrete behaviour for these functions, then
 * when syscall come, pc will konw its syscall number and can invoke
 * the specific function to handle the syscall
*/
  pfn[SYS_HALT] = syscall_HALT;
  pfn[SYS_EXIT] = syscall_EXIT;
  pfn[SYS_EXEC] = syscall_EXEC;
  pfn[SYS_WAIT] = syscall_WAIT;
  pfn[SYS_CREATE] = syscall_CREATE;
  pfn[SYS_REMOVE] = syscall_REMOVE;
  pfn[SYS_OPEN] = syscall_OPEN;
  pfn[SYS_FILESIZE] = syscall_FILESIZE;
  pfn[SYS_READ] = syscall_READ;
  pfn[SYS_WRITE] = syscall_WRITE;
  pfn[SYS_SEEK] = syscall_SEEK;
  pfn[SYS_TELL] = syscall_TELL;
  pfn[SYS_CLOSE] = syscall_CLOSE;
}

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  if (!is_valid_addr(f->esp))
  {
    process_exit_with_status(-1);
  }
  int No = *(int *)(f->esp);
  if (No >= MAXCALL || No < 0)
  {
    // printf("We don't have this system call!\n");
    process_exit_with_status(-1);
  }
  if (pfn[No] == NULL)
  {
    printf("have not implemenyt this system call\n");
    process_exit_with_status(-1);
  }
  f->eax = pfn[No](f); // put return value of syscall in f->eax, default return value is 0
  // printf ("system call!\n");
  // thread_exit ();
}

// this function make current thread exit itself with exit code: status
void process_exit_with_status(int status)
{
  struct thread *cur = thread_current();
  cur->rtv = status;
  thread_exit();
}

int syscall_HALT(struct intr_frame *f) /* Halt the operating system. */
{
  shutdown_power_off();
  return 0;
}

int syscall_EXIT(struct intr_frame *f) /* Terminate this process. */
{
  int *esp = (int *)f->esp;
  int rtv;
  pop_stack(esp, &rtv, 1);
  // if (!is_valid_addr(esp + 1))
  // {
  //   process_exit_with_status(-1);
  // }
  // else
  // {
  // int rtv = *(esp + 1);
  process_exit_with_status(rtv);
  // }
  return 0;
}

int syscall_EXEC(struct intr_frame *f) /* Start another process. */
{
  int *esp = (int *)f->esp;
  char *file_name;
  pop_stack(esp, &file_name, 1);
  if( !is_valid_addr(file_name) ){
    process_exit_with_status(-1);
    return -1;
  }

  if( !is_valid_addr(file_name+1) ){
    printf("error\n");
  }

  return process_execute(file_name);

  /* Open executable file and check if it is exist */
  // char * name_copy = malloc (strlen(file_name)+1);
  // char* argument_ptr;
	// strlcpy(name_copy, file_name, strlen(file_name) + 1);


  // name_copy = strtok_r(name_copy, " ", &argument_ptr);
  // struct file* file = filesys_open (name_copy);
  // if (file == NULL) 
  //   {
  //     process_exit_with_status(-1);
  //     return 0;
  //   }else{
  //     file_close(file);
  //     return process_execute(file_name);
  //   }
  //   free(name_copy);

}

int syscall_WAIT(struct intr_frame *f) /* Wait for a child process to die. */
{
  int *esp = (int *)f->esp;
  // if(!is_user_vaddr(esp+7))
  // ExitStatus(-1);
  tid_t wait_id;
  pop_stack(esp, &wait_id, 1);
  return process_wait(wait_id);
}

int syscall_CREATE(struct intr_frame *f) /* Create a file. */
{
	int ret;
	off_t initial_size;
	char *name;

	pop_stack(f->esp, &initial_size, 2);
	pop_stack(f->esp, &name, 1);
	if (!is_valid_addr(name))
		ret = -1;

	ret = filesys_create(name, initial_size);
	return ret;
}

int syscall_REMOVE(struct intr_frame *f) /* Delete a file. */
{
}

int syscall_OPEN(struct intr_frame *f) /* Open a file. */
{
	int ret;
	char *name;

	pop_stack(f->esp, &name, 1);
	if (!is_valid_addr(name))
		ret = -1;

	// lock_acquire(&filesys_lock);
	struct file *fptr = filesys_open(name);
	// lock_release(&filesys_lock);

	if (fptr == NULL)
		ret = -1;
	else
	{
		struct process_file *pfile = malloc(sizeof(*pfile));
		pfile->ptr = fptr;
		pfile->fd = thread_current()->fd_count;
		thread_current()->fd_count++;
		list_push_back(&thread_current()->opened_files, &pfile->elem);
		ret = pfile->fd;
	}
	return ret;
}

int syscall_FILESIZE(struct intr_frame *f) /* Obtain a file's size. */
{
  int ret;
	int fd;
	pop_stack(f->esp, &fd, 1);

	// lock_acquire(&filesys_lock);
	ret = file_length (search_fd(&thread_current()->opened_files, fd)->ptr);
	// lock_release(&filesys_lock);

	return ret;
}
struct process_file *
search_fd(struct list* files, int fd)
{
	struct process_file *proc_f;
	for (struct list_elem *e = list_begin(files); e != list_end(files); e = list_next(e))
	{
		proc_f = list_entry(e, struct process_file, elem);
		if (proc_f->fd == fd)
			return proc_f;
	}
	return NULL;
}

int syscall_READ(struct intr_frame *f) /* Read from a file. */
{
  int ret;
	int size;
	void *buffer;
	int fd;

	pop_stack(f->esp, &size, 3);
	pop_stack(f->esp, &buffer, 2);
	pop_stack(f->esp, &fd, 1);

	if (!is_valid_addr(buffer))
		ret = -1;

	if (fd == 0)
	{
		int i;
		uint8_t *buffer = buffer;
		for (i = 0; i < size; i++)
			buffer[i] = input_getc();
		ret = size;
	}
	else
	{
		struct process_file *pf = search_fd(&thread_current()->opened_files, fd);
		if (pf == NULL)
			ret = -1;
		else
		{
			ret = file_read(pf->ptr, buffer, size);
		}
	}

	return ret;
}

int syscall_WRITE(struct intr_frame *f) /* Write to a file. */
{
  int *esp = (int *)f->esp;
  // if(!is_user_vaddr(esp+7))
  // ExitStatus(-1);
  int fd;
  char *buffer;
  unsigned int size;
  int ret;
  pop_stack(esp, &fd, 1);
  pop_stack(esp, &buffer, 2);
  pop_stack(esp, &size, 3);
  // unsigned int size = *(esp + 3);

  if (fd == 1) // means it write to console (stdout)
  {
    putbuf(buffer, size);
    f->eax = 0;
  }else{
    struct process_file *pf = search_fd(&thread_current()->opened_files, fd);
    if (pf == NULL)
			ret = -1;
		else
		{
			ret = file_write(pf->ptr, buffer, size);
		}
  }
  return ret;
}

int syscall_SEEK(struct intr_frame *f) /* Change position in a file. */
{
  
}

int syscall_TELL(struct intr_frame *f) /* Report current position in a file. */
{
}
int syscall_CLOSE(struct intr_frame *f) /* Close a file. */
{
}
