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

