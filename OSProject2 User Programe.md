# OSProject2 User Programe
member: chengmin, huang Yu'an

### Before start:
1. change --qumu to --bocus,

2. process_wait() and system call write need to implement first, unless nothing output. (can use semaphore), now its busy waitting. It needs more smart implement later especially for syscall `wait()`. 

3. Can change compile optimized model to O0 for more convenient debug.

### Start:

Need to write system call hander for each system call.

must use argument -v -k -t 60



```c
 /* We arrive here whether the load is successful or not. */
  //=============we need to make sure no one can change the elf 
  file_deny_write (file);

  t->proc.this_file = file;
  // file_close (file);
```





### Test

- halt: implement HALT
- exit: implement Exit
- sc-bad-sp: check f->esp's value, but I can not recognize this type of bad address.
- sc-bad-arg: check f->esp's value if is valid.
- exec-bound-2: check esp's value, but I can not kown whether this address is bad!!
- exec-bound-3: see exec-bound-2.



- bad-read: can not read/right/jump address unmapped, add some code in exception.c

```c
 if(!user)
  printf ("Page fault at %p: %s error %s page in %s context.\n",
          fault_addr,
          not_present ? "not present" : "rights violation",
          write ? "writing" : "reading",
          user ? "user" : "kernel");


case SEL_UCSEG:
      /* User's code segment, so it's a user exception, as we
         expected.  Kill the user process.  */

      //========just exit current process============
      process_exit_with_status(-1); // this function make current thread exit itself with exit code: status
```



- bad-write
- bad-read2
- bad-write2
- bad-jump
- bad-jump2
- syn-read: 
  
  - what does child-syn-read do?  problem: child finished first, so when father wait it it directly    
  
    return -1 immediately.
  
    find wait bug and fix it

```c

/**
 * struct process, which is in strcut thread, store a semaphore to implement wait(), 
 * and also store all information about this process such as open file, child process
 * 
*/
struct process
{
  struct semaphore wait; // to implement wait a specially child, father will wait in this semaphore
  struct semaphore wait_anyone; // to implement wait(-1)
  struct semaphore wait_load; // to implement wait load, father need to wait child process loaded completely

  struct thread* father;
  
  struct list child;
  // struct list_elem child_elem; // list elem for child list.
  tid_t pid;
  bool is_loaded;

  struct file* this_file; // store the excutable file itself
  int rtv;             // return value of this thread(process).

  /* data */
};

struct process_node
{
  struct semaphore* father_wait; // its father will wait in this semaphore
  // struct semaphore wait_anyone; // to implement wait(-1)
  // struct semaphore wait_load; // to implement wait load, father need to wait child process loaded completely

  struct list_elem child_elem; // list elem for child list.
  tid_t pid;
  // bool is_loaded;

  int rtv;             // return value of this thread(process).
};

```





### The things we need to attension

- need to synchronized when process_start() load the executable file since there may be more than 1 process want to execute the same file.

### 1.3 Synchronization

During the function `laod`, we allocate page directory for the file, open the file, push the argument into the stack. According to **Task3**, the file operation syscalls do not call multiple filesystem functions concurrently. Therefore, we have to keep the file from modified or opened by other processes. We implement it by using `filesys_lock` (defined in `thread.h`which we will explain in **Task3**):

```
lock_acquire(&filesys_lock);
//loading the file
lock_release(&filesys_lock);
```

Also, according to the **Task3**, while a user process is running, the operating system must ensure that nobody can modify its executable on disk. `file_deny_write(file)` denies writes to this current-running files.



The invalid memory access include:

- NULL pointer
- Invalid pointers (which point to unmapped memory locations)
- Pointers to the kernel’s virtual address space



##### 2.2.2.1 Check valid address

We implement it in function:

```
void *is_valid_addr(const void *vaddr)
{
	void *page_ptr = NULL;
	if (!is_user_vaddr(vaddr) || !(page_ptr = pagedir_get_page(thread_current()->pagedir, vaddr)))
	{
		exit_process(-1);
		return 0;
	}
	return page_ptr;
}
```

**pop_statck():**









### Write a syscall


when wiite the callback function for syscall, we need to add its implement (as well as resister) in `syscall_init` which is in userprog/syscall.c (most of code we need to write in this project is in this file).

```c

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
  pfn[SYS_HALT] = IHALT;
  pfn[SYS_EXEC] = IEXEC;
  pfn[SYS_WAIT] = IWAIT;
}
```

This function call current thread to exit will `status` as exit code. I add a atrribute `rtv: int` in struct thread as its return value.   

```c
void ExitStatus(int status){
  struct  thread* cur = thread_current();
  cur -> rtv = status;
  thread_exit();
}
```

The syscall parameter arguments was stored in `f->esp`. The order and type of them can be seen in lib/user/syscall.c.
The return value of syscall was put in `f->eax`.

```c
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

```



**process_wait**:

add struct process in thread.h:

```c
struct process
{
  struct semaphore wait; // to implement wait a specially child, father will wait in this semaphore
  struct semaphore wait_anyone; // to implement wait(-1)
  struct semaphore wait_load; // to implement wait load, father need to wait child process loaded completely

  struct thread* father;
  
  struct list child;
  struct list_elem child_elem; // list elem for child list.
  tid_t pid;
  bool is_loaded;
  /* data */
};

```







```c
int
process_wait (tid_t child_tid UNUSED) 
{ 
  // =======check if I am your father=========
  if( !is_child(child_tid) || child_tid==TID_ERROR ){
    return -1;
  }
  if(child_tid == -1){
    struct thread* cur = thread_current();
    sema_down( &(cur->proc).wait_anyone );
    return -1;
  }

  // =============get the wait child's samephore=======
  struct thread* t = find_thread_by_tid(child_tid);
  if(t->status==THREAD_DYING){
    return -1;
  }

  struct semaphore* to_wait = &(t->proc).wait;
  sema_down(to_wait);

  // thread_set_priority(PRI_MIN);
  /*
  if(to_wait!=NULL){
    to_wait->value = 0;
    sema_down(to_wait);
  }  
  struct thread* t = find_thread_by_tid(child_tid);
  while( t->tid==child_tid && t->status!=THREAD_DYING ){
    thread_yield();
  }
  // =========now just busy waiting===============
  */
  return t->rtv;
}
```

