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
#define NAME_PID_LEN 20

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
	//printf ("mode = %d\n", mode);
    if (mode == 0) // streamer
    {
        if (argc == 2)
        {
            printf ("no file\n");
            return 0;
        }

        char followerName [NAME_PID_LEN] = {};
        int rdQueFd = open ("readersque", O_RDWR);

        int readRet = read (rdQueFd, followerName, NAME_PID_LEN);

        //if a process dies here other end won't close

        int fdTo = open (followerName, O_WRONLY);
        if (errno == ENXIO)
        {
        	printf ("errno == ENXIO\n");
            return -1;
        }

        char buffer [BUFFER_LENGHT] = {};

        int fdFrom = open (argv [2], 0);

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
        close (rdQueFd);
        return 0;

    }
    else if (mode == 1) // follower
    {
        if (argc == 3)
        {
            printf ("I dont need any more arguments in this mode\n");
            return 0;
        }

        char namePid [NAME_PID_LEN] = {};
        sprintf (namePid, "%d", getpid());
        mkfifo (namePid, 0644);

		//
        int rdQueFd = open ("readersque", O_WRONLY);
        write (rdQueFd, namePid, NAME_PID_LEN);
		//


        int fdFrom = open (namePid, O_RDONLY);
		//fcntl (fdFrom, F_SETFL, O_RDONLY | O_NONBLOCK);


        char buffer [BUFFER_LENGHT] = {};

        int readRetValue = BUFFER_LENGHT;

        while (readRetValue == BUFFER_LENGHT)
        {
            readRetValue = read (fdFrom, buffer, BUFFER_LENGHT);
            if (readRetValue > 0)
                write (STDOUT_FILENO, buffer, readRetValue);
        }


        close  (fdFrom);

        close (rdQueFd);
		unlink (namePid);
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

//-------------------------------------------------------------------


