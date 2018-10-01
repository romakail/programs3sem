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
    if (mode == 0) // writes
    {
        if (argc == 2)
        {
            printf ("no file\n");
            return 0;
        }

        char buffer [BUFFER_LENGHT] = {};

        int fdFrom = open (argv [2], 0);
        int fdTo   = open ("fifopipe", O_WRONLY);

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


		char permBuffer [2] = {};
        int fdReadPerm = open ("readperm", O_RDWR);
        int readPermVal = read (fdReadPerm, permBuffer, 2);



        if (readPermVal == 0)
		{
			printf ("File is empty!!\n");
			close (fdReadPerm);
			return 0;
		}
		else if (readPermVal == 1)
		{
			if (permBuffer[0] == '1')
			{
				close (fdReadPerm);
				printf ("Someone is reading, I dont want to elbow out\n");
				return 0;
			}
			else if (permBuffer[0] == '0')
			{
				char buffer [BUFFER_LENGHT] = {};

				int fdFrom = open ("fifopipe", O_RDONLY);

				int readRetValue = 1;

				while (readRetValue > 0)
				{
					readRetValue = read (fdFrom, buffer, BUFFER_LENGHT);
					if (readRetValue > 0)
						write (STDOUT_FILENO, buffer, readRetValue);
				}
				close (fdFrom);

				readPermVal = read (fdReadPerm, permBuffer, 1);
				lseek (fdReadPerm, 0, SEEK_SET);

				write (fdReadPerm, "0", 1);
			}
		}
		else
		{
            close (fdReadPerm);
            printf ("MISTAKE, readPermVal > 1");
            return 0;
		}
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




