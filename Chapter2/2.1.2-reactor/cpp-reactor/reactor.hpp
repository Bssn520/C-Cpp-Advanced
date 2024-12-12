#include <string>

// using 别名 = 返回类型 (*)(参数列表);
using RCALLBACK = int (*)(int fd);

struct connection_item
{
    int fd;
    std::string wbuf;
    int wlength;
    std::string rbuf;
    int rlength;
    union
    {
        RCALLBACK accept_callback;
        RCALLBACK recv_callback;
    } recv_t;
    RCALLBACK send_callback;
};