/*
setjmp/longjmp 实现函数之间的跳转
*/

#include <setjmp.h>
#include <stdio.h>

jmp_buf env;

int func(int arg)
{
    printf("arg: %d\n", arg);
    // longjmp 接受两个参数：第一个参数是 jmp_buf 类型的变量，用于指定要恢复的环境；第二个参数是一个整数值，这个值将作为 setjmp 的返回值。
    // 调用 longjmp 后，程序的执行会跳回到 setjmp 调用的地方，并且 setjmp 会返回 longjmp 指定的值。
    longjmp(env, ++arg); // setjmp
}

int main(int argc, const char **argv)
{
    // setjmp 的返回值有两种可能：第一次调用时，setjmp 返回 0；当 longjmp 被调用时，setjmp 会返回 longjmp 指定的值（非零值）。
    int ret = setjmp(env); // 第一次调用的返回值为 0
    if (ret == 0)
    {
        func(ret);
    }
    else if (ret == 1)
    {
        func(ret);
    }
    else if (ret == 2)
    {
        func(ret);
    }
    else if (ret == 3)
    {
        func(ret);
    }

    return 0;
}
