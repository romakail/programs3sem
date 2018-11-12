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
#define MAX_SLEEP_TIME 10

#define PRINT(args...)    \
    printf(args);      \
    fflush (stdout);        \


int modeDetection (char* mode);
int follower ();
int streamer (char* fileFromName);

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

        streamer(argv[2]);

    }
    else if (mode == 1) // follower
    {
        if (argc == 3)
        {
            printf ("I dont need any more arguments in this mode\n");
            return 0;
        }

        follower ();
    }
    else
    {
		printf ("some mistake occured\n");
		return 0;
    }

    return 0;
}

//-----------------------------------------------------------------------------------------

int follower ()
{
    PRINT ("hey, I am follower\n")
    char streamerName [NAME_PID_LEN] = {};

    int rdQueFd = open ("readersque", O_RDWR);
    if (rdQueFd == -1)
    {
        printf ("error with opening readersque\n");
        return -1;
    }

    int readRet = read (rdQueFd, streamerName, NAME_PID_LEN);
    PRINT ("Got PID from the queue\nPID = %s\n", streamerName)
                                                                    //-----got pid from the queue
    //int fdFrom = open (streamerName, O_RDONLY | O_NONBLOCK);
    int fdFrom = open (streamerName, O_RDONLY);
    PRINT ("opened for writeonly, fdFrom = %d\n", fdFrom)
                                                                    //-----we are being waited on the other side
    //fcntl (fdFrom, F_SETFL, O_RDONLY);
                                                                    //-----for proper communication

                                                                    //-----starting to write
    char buffer [BUFFER_LENGHT] = {};

    int readRetValue = BUFFER_LENGHT;

    while (readRetValue == BUFFER_LENGHT)
    {
        readRetValue = read (fdFrom, buffer, BUFFER_LENGHT);
        if (readRetValue > 0)
            write (STDOUT_FILENO, buffer, readRetValue);
    }
                                                                    //-----finishing to write

    close  (fdFrom);
    close (rdQueFd);

    unlink (streamerName);

    return 0;
}

//------------------------------------------------------------------------------------------------------

int streamer (char* fileFromName)
{
    PRINT ("hey, I am streamer\n")
    int fdFrom = open (fileFromName, 0);
    if (fdFrom == -1)
    {
        printf ("file do not exists\n");
        return -1;
    }
                                                            //-----opened the fileFrom

    char namePid [NAME_PID_LEN] = {};
    sprintf (namePid, "%d", getpid());
    mkfifo (namePid, 0644);
    PRINT ("made a fifo, name = %s\n", namePid);
                                                            //-----created a pidFifo
    int rdQueFd = open ("readersque", O_WRONLY);
    write (rdQueFd, namePid, NAME_PID_LEN);
    PRINT ("wrote pid into the queue\n")
                                                            //-----wrote pid into the queue
    int fdTo = 0;
    int sleepTime = 0;
    do
    {
        sleep (sleepTime);
            PRINT ("I am in busy wait, sleepTime = %d\n", sleepTime)
            PRINT ("namePid = \"%s\"", namePid)
        if (errno == ENXIO)
            printf ("errno = ENXIO\n");
        fdTo = open (namePid, O_WRONLY | O_NONBLOCK);
            PRINT ("fdTO = %d\n", fdTo)
        if (sleepTime >= MAX_SLEEP_TIME)
        {
            printf ("Sorry I have slept to much\n");
            printf ("Nobody opened from the other side\n");
            return -1;
        }
        sleepTime++;

    } while ((fdTo == -1) && (sleepTime <= MAX_SLEEP_TIME));
                                                            //-----end of busy wait
    fcntl (fdTo, F_SETFL, O_WRONLY);


                                                            //-----start writing
    char buffer [BUFFER_LENGHT] = {};


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

//------------------------------------------------------------------------------------------------------

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







/*

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
        if (rdQueFd == -1)
        {
            printf ("error with opening readersque\n");
            return -1;
        }

        int readRet = read (rdQueFd, followerName, NAME_PID_LEN);


        //if a process dies here other end won't close

        //int fdTo = open (followerName, O_WRONLY);
        int fdTo = open (followerName, O_WRONLY | O_NONBLOCK);
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



*/
