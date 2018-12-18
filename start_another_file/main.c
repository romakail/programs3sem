#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc != 0)
    {
        printf ("something wrong with argv pointer\n");
        return 0;
    }

    execv (argv[1], &(argv[1]));
    return 0;
}
