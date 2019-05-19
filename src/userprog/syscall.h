#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#define MAXCALL 20
#include "kernel/list.h"

typedef void(*CALL_PROC)(struct intr_frame*);
void syscall_init (void);


//====this struct store the file this process hold=======
struct process_file {
	struct file* ptr;
	int fd;
	struct list_elem elem;
};
void process_exit_with_status(int status); // this function make current thread exit itself with exit code: status
void release_all_file( );



extern struct lock file_read_write_lock;

#endif /* userprog/syscall.h */
