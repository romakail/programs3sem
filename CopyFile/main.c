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

#define PRINT(args...)      \
    printf(args);           \
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


    char namePid [NAME_PID_LEN] = {};
    sprintf (namePid, "%d", getpid());
    mkfifo (namePid, 0644);

    int fdFrom = open (namePid, O_RDONLY | O_NONBLOCK);
    if (fdFrom == -1)
    {
        printf ("some error occured with opening Fifo for RD_ONLY | O_NONBLOCK, pid = %s\n", namePid);
        return -1;
    }

    int folQueFd = open ("followersqueue", O_RDWR);
    write (folQueFd, namePid, NAME_PID_LEN);

    int sleepTime = 0;
    char buffer [BUFFER_LENGHT] = {};
    int readRetValue = 0;

    //===========================================================================

    do
    {
        sleep (sleepTime);
        readRetValue = read(fdFrom, buffer, BUFFER_LENGHT);
        sleepTime++;
    } while ((readRetValue == 0) && (sleepTime <= MAX_SLEEP_TIME + 1));

    //===========================================================================

    if (readRetValue == 0)
    {
        printf ("file with name /'%s/' was not opened on the other side\n", namePid);
        return -1;
    }

    fcntl(fdFrom, O_RDONLY);

    while (readRetValue != 0)
    {
        write (STDOUT_FILENO, buffer, readRetValue);
        readRetValue = read (fdFrom, buffer, BUFFER_LENGHT);
    }

    close (fdFrom);
    close (folQueFd);

    unlink (namePid);

    return 0;
}

//------------------------------------------------------------------------------------------------------

int streamer (char* fileFromName)
{
    int fdFrom = open (fileFromName, 0);
    if (fdFrom == -1)
    {
        printf ("file do not exists\n");
        return -1;
    }

    int folQueFd = open ("followersqueue", O_RDWR);
    if (folQueFd == -1)
    {
        printf ("error with opening followersqueue\n");
        return -1;
    }

    char followerName [NAME_PID_LEN] = {};

    //--------------------Start of a critical section---------------------------

    int folReadRet = read (folQueFd, followerName, NAME_PID_LEN);
    if (folReadRet == -1)
    {
        printf ("some error with getting followers pid from main pipe\n");
        printf ("read returned %d\n", folReadRet);
        return -1;
    }

    //--------------------Finish of a critical section---------------------------


    int fdTo = open (followerName, O_WRONLY | O_NONBLOCK);
    if(fdTo < 0)
    {
        perror("fd on the other side has died\n");
        return -1;
    }


    fcntl (fdTo, O_WRONLY);

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

    close (fdTo);
    close (fdFrom);

    close (folQueFd);

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
