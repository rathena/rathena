# How to use `debugbreak-gdb.py`

Just add `-x debugbreak-gdb.py` to your usual GDB invocation or add `source debugbreak-gdb.py` to your `$HOME/.gdbinit`.

Here's a sample session:

```
$ cd debugbreak
$ make
$ gdb -q -x debugbreak-gdb.py test/break
Reading symbols from test/break...done.

(gdb) set disassemble-next-line 1
(gdb) run
Starting program: /home/fedora/debugbreak/test/break 

Program received signal SIGTRAP, Trace/breakpoint trap.
main () at test/break.c:6
6		debug_break();

Program received signal SIGTRAP, Trace/breakpoint trap.
main () at test/break.c:6
6		debug_break();
(gdb) debugbreak-step
7		printf("hello world\n");
(gdb) debugbreak-continue 
hello world
[Inferior 1 (process 12533) exited normally]
(gdb)

```

On ARM and POWER, trying to use `step` or `stepi` in place of `debugbreak-step` in the sesion above wouldn't have worked as execution would be stock on the breakpoint instruction.
