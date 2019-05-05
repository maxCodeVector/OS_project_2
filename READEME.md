change --qumu to --bocus,

process_wait() and `system call write` need to implement first, unless nothing output. 

(can use semaphore, now just use busy waiting, it can not solve  wait(-1) problem ).

Need to write system call hander for each system call.

must use argument -v -k -t 60

- halt:  implement HALT

- exit: implement Exit

- sc-bad-sp: check f->esp's value, but I can not recognize this type of bad address.

- sc-bad-arg: check f->esp's value

