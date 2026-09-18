# Lab 1: System calls & LKMs

## Introduction
In this assignment we will implement two system calls in the Linux kernel, and create a loadable kernel module. This assignment will be completed on your Cloud VM, and will require compiling the kernel at least twice. The steps needed to complete this assignment include:
Lab1:
- [Hello World system call](#hello-world-system-call)
- [Test program](#test-program)
Lab2:
- [Multiplication system call](#multiplication-system-call)
- [Creating an LKM](#creating-a-loadable-kernel-module)

## Hello World system call
In this first portion, you will add a new system call, recompile the kernel, and run the kernel with the added system call. First, navigate to the directory where your kernel sources live, create the file `arch/x86/kernel/helloworld.c` and insert the following code:
```c
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(helloworld) {
    printk(KERN_ALERT "Hello World\n");
    return 0;
}
```
Some notes on the above code:
- The `kernel.h` header file contains constants and functions used in kernel programming
- The `linkage.h` header file defines [macros](https://en.wikipedia.org/wiki/C_preprocessor) that are used to keep the stack safe and ordered
- The `syscalls.h` header file contains the `SYSCALL_DEFINEn` macros, where `n` is the number of parameters being passed to the system call. Since `helloworld()` expects zero parameters, we use `SYSCALL_DEFINE0`.
- The function [`printk()`](https://www.kernel.org/doc/html/latest/core-api/printk-basics.html) is used to print messages to the kernel log (`dmesg`). The `KERN_ALERT` macros specifies the message to be printed in red text. It is important to note that it is **critical** that messages are terminated with a newline (`\n`) character.

Next, we have to tell the build system about our new function. Edit the file `arch/x86/kernel/Makefile`. Inside, you will see a large number of lines that begin with `obj-`. At the end of this list (right before the second on **64-bit specific files**), add the following:
```make
obj-y += helloworld.o
```
Here, the build system is informed that a `helloworld.o` file needs to be compiled into the kernel, and it will come from a `helloworld.c` file.

Now, our function needs to be assigned a system call number known to kernel, and later on, as a way to call our function. Edit the `arch/x86/entry/syscalls/syscall_64.tbl` file, and add your new system call at the next numerical entry in the 64-bit section:
```
463 common helloworld sys_helloworld
```
Here, the first parameter is the system call number, followed by the type of system call (common in our case), the function name itself, and the function name prefixed with `sys_`, as it will be known to the kernel.

Next, add the new system call prototype in the system call header file that we included previously. Edit the `include/linux/syscalls.h` file and add the following for your system call at the end of the file, just **before** the last `#endif` statement:
```c
asmlinkage long sys_helloworld(void);
```
The [`asmlinkage`](https://kernelnewbies.org/FAQ/asmlinkage) keyword tells the compiler that the function should not expect to find any of its arguments in registers, and should look directly to the stack. This macro is defined in the `linkage.h` header file. [not useful for this one]

Finally, recompile, install, and boot into the modified kernel using the same steps from the prior assignment:
```
$ make -j8
$ sudo make modules_install
$ sudo make install
$ sudo update-grub
$ sudo reboot
```
If the above completed without errors, you should now boot into your new kernel and proceed to the next step.

## Test program
Once in the new kernel, you should be able to use the new system call. We will create a user space test program to verify the functionality of the `helloworld()` system call.
```c
#include <unistd.h>
#include <stdlib.h>

int main() {
    long res = syscall(463);
    return res;
}
```
The first argument to the [`syscall()`](https://man7.org/linux/man-pages/man2/syscall.2.html) function is the system call number we assigned previously. Additional arguments would be passed to the function being called. If successful, a system call returns 0, otherwise it returns -1.
```shell
$ gcc hello_test.c
$ ./a.out
$ echo $?
0
```
The `echo` command above is used to print the return status of the previous command executed. Now that the system call has been invoked, the message we printed should be shown in the kernel log:
```shell
$ sudo dmesg
[sudo] password for student: 
[   16.130924] vmxnet3 0000:03:00.0 ens160: intr type 3, mode 0, 9 vectors allocated
[   16.132015] vmxnet3 0000:03:00.0 ens160: NIC Link is Up 10000 Mbps
[   17.285672] loop18: detected capacity change from 0 to 8
[   20.642405] rfkill: input handler disabled
[  107.365616] kauditd_printk_skb: 37 callbacks suppressed
[  107.584111] rfkill: input handler enabled
[  109.652854] rfkill: input handler disabled
[  225.390222] Hello World
```
Note, there are many, many applications and services printing to the kernel log, thus you will likely need to look at its output fairly quickly after calling your system call for it to be seen.

## Lab 2: Multiplication system call
Once your Hello World system call is functioning, you will now create a second system call. Name this new system call `csci3753_mult()`. This function will take three arguments: two integers `number1` and `number2`, and a long pointer `result`.
```c
SYSCALL_DEFINE3(csci3753_mult, int, number1, int, number2, long *, result)
```
This system call must do the following:
- Use `printk()` to log the two numbers being multiplied
- Multiply those two numbers
- Use `printk()` to log the result
- Store the result in user space via the long integer pointer argument
    - Ensure you account for the fact that the kernel **cannot** directly write to a user space memory location
- Return with an appropriate status; 0 indicates success, and a non-zero value indicates failure

Follow the same steps you took for the Hello World system call to recompile and install the kernel a second time. Once again, you should write a user space test program to verify the system call works appropriately:
```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SYSCALL_NUM 463

int main(int argc, char **argv) {
    if (argc < 3)
        return -1;
    
    int a = atoi(argv[1]); // multiplicand
    int b = atoi(argv[2]); // multiplier
    long c;                // product

    long res = syscall(SYSCALL_NUM, a, b, &c);

    printf("%d * %d = %ld\n", a, b, c);
    return res;
}
```

### Hints
You can compile just your system call code with:
```
$ make arch/x86/kernel/<file>.o
```
This will help identify any syntactical errors in your code without recompiling the entire kernel. Do note that this does **not** omit the need to recompile and reinstall the kernel for the system call to actually be run.

## Creating an LKM
As we've seen, if you make changes to the kernel, you must perform the tedious task of compiling, install, and booting the new kernel for these changes to take affect. An alternative to this is loadable kernel modules, which allow (mostly) the same functionality to the kernel without the need to recompile or reboot. In short, LKMs are object files (`.ko` - kernel object) that can be used to extend a running kernel's functionality on the fly.

To get started, create a new directory `/home/kernel/modules` and a file named `hello_module.c` containing the following code:
```c
#include <linux/init.h>
#include <linux/module.h>

MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");

int hello_init(void) {
    printk(KERN_ALERT "Inside %s function\n", __FUNCTION__);
    return 0;
}

void hello_exit(void) {
    printk(KERN_ALERT "Inside %s function\n", __FUNCTION__);
}

module_init(hello_init);
module_exit(hello_exit);
```
In the above code:
- The `linux/{init.h,module.h}` header files contain library headers for module initialisation and other functions that support LKMs
- The `module_init()` and `module_exit()` functions belong to the kernel, and bind *our* `hello_init()` and `hello_exit()` functions to be executed when the module is installed and removed
- As this code is still running in the kernel space, we cannot use user space functions like `printf()`, thus `printk()` is still needed to display messages in the kernel log

In this same directory, create a file named `Makefile` with the following code:
```make
obj-m := hello_module.o
```
In this line, `obj-m` means module, and the line as a whole tells the compiler to create a module object named `hello_module.o`. To compile the module into something usable, issue the following command:
```
$ make -C /lib/modules/$(uname -r)/build M=$PWD
```
If successful, the following files should have been made:
```
$ ls -l
-rw-rw-r-- 1 user user    332 Sep  1 23:27 hello_module.c
-rw-rw-r-- 1 user user 209832 Sep  1 23:34 hello_module.ko
-rw-rw-r-- 1 user user     36 Sep  1 23:34 hello_module.mod
-rw-rw-r-- 1 user user    816 Sep  1 23:34 hello_module.mod.c
-rw-rw-r-- 1 user user 105776 Sep  1 23:34 hello_module.mod.o
-rw-rw-r-- 1 user user 105312 Sep  1 23:34 hello_module.o
-rw-rw-r-- 1 user user     21 Sep  1 23:33 Makefile
-rw-rw-r-- 1 user user     36 Sep  1 23:34 modules.order
-rw-rw-r-- 1 user user      0 Sep  1 23:34 Module.symvers
```

Followup question: can you extend the `Makefile` such that the compilation command can be run simply by running `make`?

In the above, notice the `.ko` file. This is the kernel module that will be loaded into the running kernel. Install the module and check that it's installed:
```
$ sudo insmod hello_module.ko
$ lsmod | grep hello
hello_module            12288  0
```
Running `dmesg` should also display the message printed by your module when initialised:
```
$ sudo dmesg
[...]
[ 2643.230212] Inside hello_init function
```
Now remove the module and check the above outputs again to verify the module is no longer loaded, and the appropriate messages are in the kernel log:
```
$ sudo rmmod hello_module
$ sudo dmesg
[...]
[ 2657.344579] Inside hello_exit function
```

## Submission requirements
- `arch/x86/kernel/helloworld.c`
- `arch/x86/kernel/csci3753_mult.c`
- We will confirm the operation of your system calls and LKM by running a test program against your VM
- Ensure your VM boots, by default, into the kernel version that includes both your system calls
- Make sure all the specified files throughout this assignment are in the correct locations on your VM
- A 1 page report of things you learned, difficulties you encountered and how you overcame them, comments about the assignment, etc.
