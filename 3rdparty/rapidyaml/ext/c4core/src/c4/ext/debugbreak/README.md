# Debug Break

[debugbreak.h](https://github.com/scottt/debugbreak/blob/master/debugbreak.h) allows you to put breakpoints in your C/C++ code with a call to **debug_break()**:
```C
#include <stdio.h>
#include "debugbreak.h"

int main()
{
	debug_break(); /* will break into debugger */
	printf("hello world\n");
	return 0;
}
```
* Include one header file and insert calls to `debug_break()` in the code where you wish to break into the debugger.
* Supports GCC, Clang and MSVC.
* Works well on ARM, AArch64, i686, x86-64, POWER and has a fallback code path for other architectures.
* Works like the **DebugBreak()** fuction provided by [Windows](http://msdn.microsoft.com/en-us/library/ea9yy3ey.aspx) and [QNX](http://www.qnx.com/developers/docs/6.3.0SP3/neutrino/lib_ref/d/debugbreak.html).

**License**: the very permissive [2-Clause BSD](https://github.com/scottt/debugbreak/blob/master/COPYING).

Known Problem: if continuing execution after a debugbreak breakpoint hit doesn't work (e.g. on ARM or POWER), see [HOW-TO-USE-DEBUGBREAK-GDB-PY.md](HOW-TO-USE-DEBUGBREAK-GDB-PY.md) for a workaround.

Implementation Notes
================================

The requirements for the **debug_break()** function are:
* Act as a compiler code motion barrier
* Don't cause the compiler optimizers to think the code following it can be removed
* Trigger a software breakpoint hit when executed (e.g. **SIGTRAP** on Linux)
* GDB commands like **continue**, **next**, **step**, **stepi** must work after a **debug_break()** hit

Ideally, both GCC and Clang would provide a **__builtin_debugtrap()** that satisfies the above on all architectures and operating systems. Unfortunately, that is not the case (yet).
GCC's [__builtin_trap()](http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-g_t_005f_005fbuiltin_005ftrap-3278) causes the optimizers to think the code follwing can be removed ([test/trap.c](https://github.com/scottt/debugbreak/blob/master/test/trap.c)):
```C
#include <stdio.h>

int main()
{
	__builtin_trap();
	printf("hello world\n");
	return 0;
}
```
compiles to:
```
main
0x0000000000400390 <+0>:     0f 0b	ud2    
```
Notice how the call to `printf()` is not present in the assembly output. 

Further, on i386 / x86-64 **__builtin_trap()** generates an **ud2** instruction which triggers **SIGILL** instead of **SIGTRAP**. This makes it necessary to change GDB's default behavior on **SIGILL** to not terminate the process being debugged:
```
(gdb) handle SIGILL stop nopass
```
Even after this, continuing execution in GDB doesn't work well on some GCC, GDB combinations. See [GCC Bugzilla 84595](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84595).

On ARM, **__builtin_trap()** generates a call to **abort()**, making it even less suitable.

**debug_break()** generates an **int3** instruction on i386 / x86-64 ([test/break.c](https://github.com/scottt/debugbreak/blob/master/test/break.c)):
```C
#include <stdio.h>
#include "debugbreak.h"
   
int main()
{
	debug_break();
	printf("hello world\n");
	return 0;
}
```
compiles to:
```
main
0x00000000004003d0 <+0>:     50	push   %rax
0x00000000004003d1 <+1>:     cc	int3   
0x00000000004003d2 <+2>:     bf a0 05 40 00	mov    $0x4005a0,%edi
0x00000000004003d7 <+7>:     e8 d4 ff ff ff	callq  0x4003b0 <puts@plt>
0x00000000004003dc <+12>:    31 c0	xor    %eax,%eax
0x00000000004003de <+14>:    5a	pop    %rdx
0x00000000004003df <+15>:    c3	retq   
```
which correctly trigges **SIGTRAP** and single-stepping in GDB after a **debug_break()** hit works well.

Clang / LLVM also has a **__builtin_trap()** that generates **ud2** but further provides **__builtin_debugtrap()** that generates **int3** on i386 / x86-64 ([original LLVM intrinsic](http://lists.llvm.org/pipermail/llvm-commits/Week-of-Mon-20120507/142621.html), [further fixes](https://reviews.llvm.org/rL166300#96cef7d3), [Clang builtin support](https://reviews.llvm.org/rL166298)).

On ARM, **debug_break()** generates **.inst 0xe7f001f0** in ARM mode and **.inst 0xde01** in Thumb mode which correctly triggers **SIGTRAP** on Linux. Unfortunately, stepping in GDB after a **debug_break()** hit doesn't work and requires a workaround like:
```
(gdb) set $l = 2
(gdb) tbreak *($pc + $l)
(gdb) jump   *($pc + $l)
(gdb) # Change $l from 2 to 4 for ARM mode
```
to jump over the instruction.
A new GDB command, **debugbreak-step**, is defined in [debugbreak-gdb.py](https://github.com/scottt/debugbreak/blob/master/debugbreak-gdb.py) to automate the above. See [HOW-TO-USE-DEBUGBREAK-GDB-PY.md](HOW-TO-USE-DEBUGBREAK-GDB-PY.md) for sample usage.
```
$ arm-none-linux-gnueabi-gdb -x debugbreak-gdb.py test/break-c++
<...>
(gdb) run
Program received signal SIGTRAP, Trace/breakpoint trap.
main () at test/break-c++.cc:6
6		debug_break();

(gdb) debugbreak-step

7		std::cout << "hello, world\n";
```

On AArch64, **debug_break()** generates **.inst 0xd4200000**.

See table below for the behavior of **debug_break()** on other architecturs.

Behavior on Different Architectures
----------------

| Architecture       | debug_break() |
| -------------      | ------------- |
| x86/x86-64         | `int3`  |
| ARM mode, 32-bit   | `.inst 0xe7f001f0`  |
| Thumb mode, 32-bit | `.inst 0xde01`  |
| AArch64, ARMv8     | `.inst 0xd4200000` |
| POWER              | `.4byte 0x7d821008` |
| RISC-V             | `.4byte 0x00100073` |
| MSVC compiler      | `__debugbreak` |
| Apple compiler on AArch64     | `__builtin_trap()` |
| Otherwise          | `raise(SIGTRAP)` |

