#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    //int nArguments = argc - 1;

    if (argc == 0)
    {
        printf ("something wrong with argv pointer\n");
        return 0;
    }


    execv (argv[1], &(argv[1]));




/*
    printf ("%s\n\n", argv[1]);

    if (argc != 2)
    {
        printf ("wrong number of argments\n");
        return 0;
    }
    else if (argc == 0)
    {
        printf ("something wrong with argv pointer\n");
        return 0;
    }

    //else if (argv[1]* == '-')
    //{
    //    printf ("number is less than zero\n");
    //    return 0;
    //}

    long val = 0;
    errno = 0;
    char* endptr = 0;

    val = strtol (argv[1], &endptr, 10);

    //printf ("%ld\n", val);

    else if (errno == ERANGE)
    {
        printf ("overflow of long int\n");
        return 0;
    }
    else if (errno == EINVAL)
    {
        printf ("no digits seen\n");
        return 0;
    }
    else if (val < 0)
    {
        printf ("less then zero");
        return 0;
    }
    else if (*endptr != '\0')
    {
        printf ("I found not a digit\n");
        return 0;
    }
    else
    {
        while (val >= 1)
        {
            printf ("%ld\n", val);
            val--;
        }
    }




    //printf ("%s\n", argv[1]);
    //printf ("%lg", strtoll(argv [1], 0, 10) )
    */
    return 0;
}
