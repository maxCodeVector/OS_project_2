#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#define MAXCALL 20

typedef void(*CALL_PROC)(struct intr_frame*);
void syscall_init (void);
// void system_write(struct intr_frame*);
#endif /* userprog/syscall.h */
