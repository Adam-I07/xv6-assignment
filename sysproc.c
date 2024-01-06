#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void) {
    return fork();
}

int sys_exit(void) {
    exit();
    return 0;  // not reached
}

int sys_wait(void) {
    return wait();
}

int sys_kill(void) {
    int pid;

    if (argint(0, &pid) < 0) {
        return -1;
    }
    return kill(pid);
}

int sys_getpid(void) {
    return myproc()->pid;
}

int sys_sbrk(void) {
    int addr;
    int n;

    if (argint(0, &n) < 0) {
        return -1;
    }
    addr = myproc()->sz;
    if (growproc(n) < 0) {
        return -1;
    }
    return addr;
}

int sys_sleep(void) {
    int n;
    uint ticks0;

    if (argint(0, &n) < 0) {
        return -1;
    }
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (myproc()->killed) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}


int sys_greeting(void) {
    cprintf("Hello again\n");
    return 0;
}


int sys_shutdown(void) {
    // Variable to store the user input
    int user_input;

    // Extract the user_input flag from the user-space argument
    argint(0, &user_input);

    // Check the value of the user_input flag
    if (user_input == 0) {
        // If user_input is 0, shut down the operating system using port 0x604
        outw(0x604, 0x2000);
    } else if (user_input == 1) {
        // If restart is 1, initiate a system restart
        // Wait for the keyboard controller to be ready
        unsigned char good = 0x02;
        while (good & 0x02) {
            good = inb(0x64);
        }
        // Send the system restart command (0xFE) to the keyboard controller
        outb(0x64, 0xFE);
    }
    
    // Return 0 to indicate success (convention in Xv6 system calls)
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}
