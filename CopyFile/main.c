#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_LENGHT 10


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

    int pipefd [2]; // 0 - read
                    // 1 - write
    pid_t pid;

    char buffer [BUFFER_LENGHT] = {};

    int pipeRet = pipe (pipefd);
    if (pipeRet == -1)
    {
        printf ("pipe returned error value\n");
        return 0;
    }

    pid = fork();
    if (pid == -1)
    {
        printf ("fork returned error value\n");
        return 0;
    }

    if (pid == 0) // child, writes
    {
        close (pipefd [0]);

        printf ("child buffer: %p\n", buffer);
        int argFileDes = 0;
        argFileDes = open (argv [1], 0);

        int  readRetValue = 1;
        int writeRetValue = 0;
        while (readRetValue != 0)
        {
            readRetValue  = read  (argFileDes, buffer, BUFFER_LENGHT);
            //printf ("[%s  readRetValue = %d]\n", buffer, readRetValue);
            if (readRetValue != 0)
            {
                writeRetValue = write (pipefd [1], buffer, readRetValue);
            }
        }

        close (pipefd [1]);
        close (argFileDes);
        //printf ("\n\n");
    }

    else // parent, reads
    {
        close (pipefd [1]);

        printf ("parrent buffer: %p\n", buffer);
        int readRetValue = 1;
        while (readRetValue > 0)
        {
            readRetValue = read (pipefd[0], buffer, BUFFER_LENGHT);
            if (readRetValue > 0)
                write (STDOUT_FILENO, buffer, readRetValue);
            //printf ("\n");
        }
        close (pipefd [0]);
        wait (NULL);
        return 0;
    }






    //printf ("%s\n", argv[1]);
    //printf ("%lg", strtoll(argv [1], 0, 10) )

    return 0;
}











/*
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
    */
