#include "syscall.h"
#include "traps.h"

.global start
start:
    pushl   $argv
    pushl   $init
    pushl   $0
    movl    $SYS_exec, %eax #执行exec系统调用
    int     $T_SYSCALL
exit:
    movl    $SYS_exit, %eax #执行exit系统调用
    int     $T_SYSCALL
    jmp     exit

init:
    .string "/init\0";
.p2align 2
argv:
    .long   init
    .long   0
