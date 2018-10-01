#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
    //printf ("%s\n\n", argv[1]);

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
    /*
    else if (argv[1]* == '-')
    {
        printf ("number is less than zero\n");
        return 0;
    }
    */
    long val = 0;
    errno = 0;
    char* endptr = 0;

    val = strtol (argv[1], &endptr, 10);

    //printf ("%ld\n", val);

    if (errno == ERANGE)
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


    printf ("val = %ld\n", val);


    /*
    for (int i = 0; i < 4; i++)
    {
        pid_t pid = fork();
        val++;
        kill(pid);
    }
    */
    //printf ("pid = %d\n", getpid());
    //printf ("val = %ld\n", val);

    /*
    pid_t pid = 1;
    pid = fork ();

    printf ("pid = %d\n", pid);
    */


    pid_t pid = 1;

    printf ("\n\n");
    printf ("superpapa = %d\n\n", getpid ());

    for (int i = 0; i < val; i++)
    {
        if (pid != 0)
        {
            pid = fork();    //papa
        }
        else
        {       //child
            printf ("n = %d  ", i);
            printf ("pid = %d  ", getpid ());
            printf ("ppid = %d\n", getppid ());
            break;
        }
        wait (0);

    }





    //printf ("%s\n", argv[1]);
    //printf ("%lg", strtoll(argv [1], 0, 10) )

    return 0;
}
