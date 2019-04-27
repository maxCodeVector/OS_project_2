# OS project 1-Thread -- Design document
黄玉安 11610303

## Task 1: Efficient Alarm Clock
The original implement is busy waiting. It is not too good. I would like to block this thread when I invoke time_sleep(). Then add some member to record how time this thread sleep. Operate System will use its clock interrupt to check the state of every thread, If it is time to weak, it will weak this thread.

###Data structure
Modify struct thread.
```
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
};
```
add a member to record the ticket this thread has been  blocked.
>  int64_t  ticks_blocked;

###Algorithms
Every ticket when clock interrupt take place it will check every thread, if its state is ``THREAD_BLOCKED``, its ticks_blocked would decrease by 1. And if the ticks_blocked goes down to 0, it will be weak by change its state to `THREAD_READY`. Since its high priority it will be schedule to running quickly. By doing this,  during the time the thread sleep, other thread with lower priority can have chance to run.
So I will have a function:
```
void check_block_thread(struct thread* thr){
	if(thr->status==THRED_BLOCKED){
		if(thr->ticks_blocked>0)
			ticks_blocked --;
		if(thr->ticks_blocked==0)
			thread_unblock (thr);
	}
}
``` 
Also the sleep function will be change, we should change
```
while (timer_elapsed (start) < ticks) 
	 thread_yield ();
```
to
>struct thread *current_thread = thread_current ();  // to get current thread 
>current_thread->ticks_blocked = ticks;
> thread_block ();

###Synchronization
Other threads might run when current thread execute the code `thread_block()`, so we need to set 
its status that can not be disturbed. To do this, we disabling Interrupts:
```
ASSERT (intr_get_level () == INTR_ON);
enum intr_level old_level = intr_disable ();
```
before setting `ticket_blocked`. And after setting `block()`, we need to recover original state, so it must run:
>intr_set_level (old_level);

The crudest way to do synchronization is to disable interrupts, that is, to temporarily prevent the CPU from responding to interrupts.

###Rationale 
The original one is implemented by `busy wait`, this implement is terrible since other threads can do nothing during the time this thread sleep. By modify to this ways, other threads have chance to run, it made good use if time. The coding is not much. It just need to add one member in struct thread and create  an additional function `check_block_thread`.


##Task 2: Priority Scheduler
By analyzing of struct thread, we see it has a member named priority. The function `schedule` will switch threads, however, it always select the first thread in the list `ready_list`. So to implement the priority schedule, we just need to make `ready_list` is a ordered list.

###Data structure 
`ready_list` is a doubled link list. It does not to be added or modified.
```
struct list_elem 
  {
    struct list_elem *prev;     /* Previous list element. */
    struct list_elem *next;     /* Next list element. */
  };

/* List. */
struct list 
  {
    struct list_elem head;      /* List head. */
    struct list_elem tail;      /* List tail. */
  };

```

And also, we need to implement priority donation for Pintos lock. To accept a donation priority, we need to add some member in struct thread. And when a thread to touch a lock, it need to know which thread has this lock, so some additional member also need to added in struct lock. 
Add this number to thread:
```
int priority_donated; // get the donated priority   
struct lock* lock_wait; // record the lock which it want to hode
```
Add this number to lock:
```
struct list_elem threads; // record threads that holds this lock 
```

###Algorithms 
The original implementation is invoke `list_push_back()` when thread run:
- thread_unblock
- init_thread
- thread_yield

This function put the current thread to the tail of `ready_list`. This is not what we expected. By looking for list.c in lib/kernel, I see a useful function: `list_insert_ordered`. 
> void list_insert_ordered (struct list *list, struct list_elem *elem, list_less_func *less, void *aux)

It will Inserts ELEM in the proper position in LIST, which must be sorted according to LESS given auxiliary data AUX. Although it runs in O(n) average case in the number of elements in LIST. So, what we need is to implement a compare function (see parameter `less`) and modify `list_push_back` to `list_insert_ordered` in above three function.

For the priority donation problem,  when a thread found it cannot get the lock, it will try to see who holds the lock and then donated its priority. Then yield itself. The compare function should think about the two kind of priority `priority` and `priority_donated ` . After a thread release a clock, it also reset its `priority_donated`. 

###Synchronization
When I operate the public `ready_list`, it must be guaranteed it will never be interrupt. And it has been down in those function. So I need not do more change, 
And also, we a thread try to donated its priority to some threads, it can not be interrupt.

###Rationale
By implement priority schedule, we can let those threads which have more important task to run first. It is what we expect in normal life and quit necessary. So the lock problem must be solved. By using priority donation, threads can finish their task normally. In this task, I add some member both in struct lock and thread. Moreover, many function need to modified such as `thread_yield, lock_acquire, lock_released` and so on. And it need to add compare function to schedule threads with thinking about two kinds of priority.


##Task 3: Multi-level Feedback Queue Scheduler
In this part, we also need priority scheduler. Since we has done this in task 2, so we need not do much in this task but need to do Multi-level Feedback Queue Scheduler (`mlfqs`) replacing priority donation.

###Data structure 
In this part, we just need to simply modify struct thread.
Add member to `struct thread`:
```cpp
/* record the recent time this thread uses cpu. And it is considered
 to float point, right 16 bits is its point part */
fixed_t recent_cpu;
int nice; // record nice value in formula, may change during run time
```
And add those code in thread's initial part:
```
t->recent_cpu = FP_CONST (0);
t->nice = 0;
```


###Algorithms 
We should use the formula:
```basic
𝑝𝑟𝑖𝑜𝑟𝑖𝑡𝑦 = 𝑃𝑅𝐼_𝑀𝐴𝑋−(𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢/4)−(𝑛𝑖𝑐𝑒×2)			[1]
		/* 𝑓(𝑡)can be a constant or some other values (like 𝑛𝑖𝑐𝑒) */
𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢(𝑡) = 𝑎 × 𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢(𝑡−1)+𝑓(𝑡) 			[2]
```
So I add those code in function `timer_interrupt` :
```cpp
update_recent_cpu_time(); // update current thread's recent cpu time
/* update all threads's recent cpu time and then modify its priority */
if (ticks % TIMER_FREQ == 4)
	update_cpu_priority();  
```
This function is called by interrupt, so it can dynamically schedule threads by change their priority. And all float point operation are using function in `pintos/src/threads/fixed-point.h`
The parameter a and function `f(t)` is to be determined.

###Synchronization 
Update recent cpu time and set thread;s priority is during clock interrupt, so we don't need to care about synchronization problem in this types implementation.

###Rationale
I implement `mlfqs` by modify the function `timer_interrupt`, it is simple and do not need to care much about shared resource problem. The code is more about to operate recent_cpu and priority which is clear.

##Answer Question

### Question in Project 1 Document
| timmer ticks | R(A)| R(B)| R(C)| P(A)| B(A)| C(A)|thread to run|
|:------------:|:----:|:--:|:----:|:--:|:----:|:----:|:----:|
| 0   |0|0|0|63|61|59|A|
| 4   |1|0|0|62|61|59|A|
| 8   |2|0|0|61|61|59|B|
| 12  |2|1|0|61|60|59|A|
| 16  |3|1|0|60|60|59|B|
| 20  |3|2|0|60|59|59|A|
| 24  |4|2|0|59|59|59|C|
| 28  |4|2|1|59|59|58|B|
| 32  |4|3|1|59|58|58|A|
| 36  |5|3|1|58|58|58|C|




###Answer questions about pintos source code

- Tell us about how pintos start the first thread in its thread system (only consider the thread part).
```cpp
static struct thread *initial_thread;

void
thread_init (void) 
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);
  list_init (&ready_list);
  list_init (&all_list);

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
}
```
use thread_init() to create the first thread and this first thread is initial _thread which is static;


- Consider priority scheduling, how does pintos keep running a ready thread with highest priority after its time tick reaching TIME_SLICE?

The `timer_interrupt()` handle will called `thread_tick()`   and if TIME_SLICE reached it enforce preemption by called `intr_yield_on_return ()`. For priority scheduling, when called `schedule()`, the higher priority thread will be run again since the ready list will make the first element to be the highest priority thread. And when system judge if the next thread need to run is current thread, it would not change its memory (by using assembly code).

- What will pintos do when switching from one thread to the other? By calling what functions and doing what?

It will called schedule(), and in this function:
```cpp
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
```
It gets the next thread that will be run,  and judge if the next is itself. If it is, it using `switch_threads`( which is implement in switch.S) to switch threads.


###How does pintos implement floating point number operation?

It implement floating point in file `pintos/src/threads/fixed-point.h`.
```
#ifndef __THREAD_FIXED_POINT_H
#define __THREAD_FIXED_POINT_H

/* Basic definitions of fixed point. */
typedef int fixed_t;
/* 16 LSB used for fractional part. */
#define FP_SHIFT_AMOUNT 16
/* Convert a value to fixed-point value. */
#define FP_CONST(A) ((fixed_t)(A << FP_SHIFT_AMOUNT))
/* Add two fixed-point value. */
#define FP_ADD(A,B) (A + B)
/* Add a fixed-point value A and an int value B. */
#define FP_ADD_MIX(A,B) (A + (B << FP_SHIFT_AMOUNT))

```
It use right 16 bit to represent floating point part and do operation by shift bit.

###What do priority-donation test cases(priority-donate-chainand priority-donate-nest)do and illustrate the running process?






