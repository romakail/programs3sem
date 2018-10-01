#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_LENGHT 10

int modeDetection (char* mode);


int main (int argc, char** argv)
{
    if ((argc > 3) || (argc == 1))
	{
        printf ("wrong number of arguments\n");
        return 0;
    }
    else if (argc == 0)
    {
        printf ("something wrong with argv pointer\n");
        return 0;
    }


	int mode = modeDetection (argv[1]);
	printf ("mode = %d\n", mode);
    if (mode == 0) // writer
    {
        if (argc == 2)
        {
            printf ("no file\n");
            return 0;
        }

        //writing your pid in the que
        int writerQueFd = open ("writerque", O_WRONLY);
        printf ("writer, writerQueFd = %d\n", writerQueFd);
        int myPid = getpid ();
        printf ("writer, myPid = %d\n", myPid);
        write (writerQueFd, &myPid, sizeof(int));
		close (writerQueFd);


        char pidBuffer [20] = {};
        sprintf (pidBuffer, "%d", myPid);
        mkfifo(pidBuffer, 0644);
        printf ("writer, made fifo, name: %s\n", pidBuffer);

        //exchange
        char buffer [BUFFER_LENGHT] = {};

        int fdFrom = open (argv [2], O_RDWR);
        printf ("writer, fdFrom = %d\n", fdFrom);
        int fdTo   = open (pidBuffer, O_WRONLY);
        printf ("writer, fdTo   = %d\n", fdTo);

		int readRetValue = 1;
        int writeRetValue = 0;
        while (readRetValue != 0)
        {
            readRetValue  = read  (fdFrom, buffer, BUFFER_LENGHT);
            if (readRetValue != 0)
            {
                writeRetValue = write (fdTo, buffer, readRetValue);
            }
        }

        close (fdFrom);
        close (fdTo);
        return 0;

    }
    else if (mode == 1) // reads
    {
        if (argc == 3)
        {
            printf ("I dont need any more arguments in this mode");
            return 0;
        }

        int writerQueFd = open ("writerque", O_RDONLY);
        printf ("reader, writerQueFd = %d\n", writerQueFd);

        int writerPid = 0;
        read (writerQueFd, &writerPid, sizeof(int));

		char fifoName [20] = {};
		sprintf (fifoName, "%d", writerPid);
		printf ("reader, fifoName = %s\n", fifoName);

		int fdFrom = -1;
		while (fdFrom == -1);
		{
        	fdFrom = open (fifoName, O_RDONLY);
        	printf ("reader, open %s returned: %d",fifoName, fdFrom);
        	sleep (1);
        }

        char buffer [BUFFER_LENGHT] = {};

        //int fdFrom = open ("fifopipe", O_RDONLY);

        int readRetValue = 1;

        while (readRetValue > 0)
        {
            readRetValue = read (fdFrom, buffer, BUFFER_LENGHT);
            if (readRetValue > 0)
                write (STDOUT_FILENO, buffer, readRetValue);
        }

        close (writerQueFd);
        close (fdFrom);
        return 0;
    }
    else
    {
		printf ("some mistake occured\n");
		return 0;
    }




    return 0;
}




int modeDetection (char* mode)
{
	int val = 0;
    errno = 0;
    char* endptr = 0;

    val = strtol (mode, &endptr, 10);

    if (errno == ERANGE)
    {
        printf ("overflow of int\n");
        return -1;
    }
    else if (errno == EINVAL)
    {
        printf ("no digits seen\n");
        return -1;
    }
    else if (val < 0)
    {
        printf ("less then zero");
        return -1;
    }
    else if (*endptr != '\0')
    {
        printf ("I found not a digit\n");
        return -1;
    }

    if ((val == 0) || (val == 1))
    	return val;
	else
		return -1;

}

























/*
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

    if (pid == 0) // child
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

    else // parent
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

*/




