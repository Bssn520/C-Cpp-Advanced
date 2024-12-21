#include <rte_eal.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
    if (rte_eal_init(argc, argv) < 0)
    {
        rte_exit(EXIT_FAILURE, "Error with EAL init\n");
    }


    return 0;
}
