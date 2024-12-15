#include <stdio.h>
#include <sys/ucontext.h>
#include <ucontext.h>

// 定义两个上下文变量 ctx[0] 和 ctx[1]，用于保存协程的上下文
ucontext_t ctx[2];

// 定义主上下文变量 main_ctx，用于保存主函数的上下文
ucontext_t main_ctx;

// 计数器，用于控制协程的执行次数
int count = 0;

// 协程函数 func1，打印 "1" 和 "3"
void func1()
{
    while (count++ < 20) // 循环执行 20 次
    {
        printf("1\n"); // 打印 "1"
        swapcontext(&ctx[0], &ctx[1]); // 切换到 ctx[1] 的上下文，即 func2
        printf("3\n"); // 打印 "3"
    }
}

// 协程函数 func2，打印 "2" 和 "4"
void func2()
{
    while (count++ < 20) // 循环执行 20 次
    {
        printf("2\n"); // 打印 "2"
        swapcontext(&ctx[1], &ctx[0]); // 切换到 ctx[0] 的上下文，即 func1
        printf("4\n"); // 打印 "4"
    }
}

int main(int argc, const char **argv)
{
    // 为协程分配栈空间
    char stack1[2048] = {0}; // 协程 func1 的栈
    char stack2[2048] = {0}; // 协程 func2 的栈

    // 获取当前上下文，保存到 ctx[0]
    getcontext(&ctx[0]);
    // 设置 ctx[0] 的栈空间
    ctx[0].uc_stack.ss_sp = stack1; // 栈的起始地址
    ctx[0].uc_stack.ss_size = sizeof(stack1); // 栈的大小
    ctx[0].uc_link = &main_ctx; // 设置后继上下文为 main_ctx，即协程结束后返回主函数
    makecontext(&ctx[0], func1, 0); // 设置 ctx[0] 的入口函数为 func1

    // 获取当前上下文，保存到 ctx[1]
    getcontext(&ctx[1]);
    // 设置 ctx[1] 的栈空间
    ctx[1].uc_stack.ss_sp = stack2; // 栈的起始地址
    ctx[1].uc_stack.ss_size = sizeof(stack2); // 栈的大小
    ctx[1].uc_link = &main_ctx; // 设置后继上下文为 main_ctx，即协程结束后返回主函数
    makecontext(&ctx[1], func2, 0); // 设置 ctx[1] 的入口函数为 func2

    printf("swapcontext\n"); // 打印 "swapcontext"
    swapcontext(&main_ctx, &ctx[0]); // 切换到 ctx[0] 的上下文，即开始执行 func1

    printf("done\n"); // 打印 "done"，表示程序结束

    return 0;
}
