#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
read_t read_f = NULL;

typedef ssize_t (*write_t)(int fd, void *buf, size_t count);
write_t write_f = NULL;

ssize_t read(int fd, void *buf, size_t count)
{
    ssize_t ret = read_f(fd, buf, count);
    printf("read: %s\n", (char *)buf);
    return ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{

    ssize_t ret = write_f(fd, (char *)buf, count);
    printf("write: %s\n", (char *)buf);
    return ret;
}

void init_hook(void)
{
    if (!read_f)
    {
        read_f = dlsym(RTLD_NEXT, "read");
    }

    if (!write_f)
    {
        write_f = dlsym(RTLD_NEXT, "write");
    }
}

int main(int argc, const char **argv)
{
    init_hook();


    return 0;
}
