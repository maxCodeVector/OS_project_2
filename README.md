# OSProject2 User Programe
member: chengmin, huang Yu'an

### Before start:
1. change --qumu to --bocus,

2. process_wait() and system call write need to implement first, unless nothing output. (can use semaphore)

3. Can change compile optimized model to O0 for more convenient test

### Start:

Need to write system call hander for each system call.

must use argument -v -k -t 60

- halt: implement HALT

- exit: implement Exit

- sc-bad-sp: check f->esp's value, but I can not recognize this type of bad address.

- sc-bad-arg: check f->esp's value

- exec-bound-2: check esp's value, but I can not kown whether this address is bad!!

- exec-bound-3: see exec-bound-2.

### Write a syscall

when wiite the callback function for syscall, we need to add its implement (as well as resister) in `syscall_init`.

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


