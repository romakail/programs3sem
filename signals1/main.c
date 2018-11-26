#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define BUFFER_LENGHT 128

int bit = 0;

int  producer (pid_t consPid, const int fdFrom);
int  consumer (pid_t prodPid);

int  prodHandlersInit ();
int  consHandlersInit ();

char receiveByte (int prodPid);
int  throwByte   (int consPid, char byte);

void usr1HandlerCons (int signal);
void usr2HandlerCons (int signal);
void emptyHandler    (int signal);

//int  sigSetInit (sigset_t* setPtr);

//------------------------------------------------------------------------------

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        printf ("wrong number of arguments\n");
		return -1;
    }

    int fdFrom = open (argv[1], O_RDONLY);
    if (fdFrom == -1)
    {
        perror ("failed to open argument\n");
        exit (0);
    }

	pid_t pid = fork ();
	if (pid == 0)				// child
	{
		producer (getppid(), fdFrom);
	}
	else						// parent
	{
		consumer (pid);
	}


    return 0;
}

//------------------------------------------------------------------------------

int producer (pid_t consPid, const int fdFrom)
{
    prodHandlersInit ();
	char buffer [BUFFER_LENGHT] = {};
	int readRet = 1;

	while (readRet > 0)
	{
		readRet = read(fdFrom, &buffer, BUFFER_LENGHT);
        for (int i = 0; i < 128; i++)
        {
            throwByte (consPid, buffer[i]);
        }
	}
	return 0;
}

//------------------------------------------------------------------------------

int consumer (pid_t  prodPid)
{
    consHandlersInit ();

    char letter = 0;

    while(1)
    {
        letter = receiveByte (prodPid);
        printf ("%c", letter);
    }

	return 0;
}

//------------------------------------------------------------------------------

int consHandlersInit ()
{
    struct sigaction action = {};
    action.sa_handler = usr1HandlerCons;
    int sigActRet = sigaction (SIGUSR1, &action, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }

    action.sa_handler = usr2HandlerCons;
        sigActRet = sigaction (SIGUSR2, &action, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }
}

//------------------------------------------------------------------------------

int prodHandlersInit ()
{
    struct sigaction action = {};
    action.sa_handler = emptyHandler;
    int sigActRet = sigaction (SIGUSR1, &action, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }

    sigActRet = sigaction (SIGUSR2, &action, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }

}

//------------------------------------------------------------------------------

int throwByte (int consPid, char byte)
{
    sigset_t emptyMask;
    sigemptyset (&emptyMask);
    printf ("---------------------------BYTE = %c\n", byte);

    for (int i = 0; i < 8; i++)
    {
        if (((1 << i) & byte) == 0)
        {
            printf ("bit = 0\n");
            kill (consPid, SIGUSR1);
        }
        else
        {
            printf ("bit = 1\n");
            kill (consPid, SIGUSR2);
        }
        sigsuspend (&emptyMask);
    }

	return 0;
}

//------------------------------------------------------------------------------

char receiveByte (int prodPid)

    sigset_t emptyMask;
    sigemptyset (&emptyMask);

	char byte = 0;

    for (int i = 0; i < 8; i++)
    {
        sigsuspend (&emptyMask);
        byte = (byte << 1) | bit;
        kill (prodPid, SIGUSR2);
    }

    printf ("poluchilos %c\n ", byte);
	return byte;
}

//------------------------------------------------------------------------------

void usr1HandlerCons (int signal)
{
    printf ("thrown 0\n");
	bit = 0;
}

//------------------------------------------------------------------------------

void usr2HandlerCons (int signal)
{
    printf ("thrown 1\n");
    bit = 1;
}

//------------------------------------------------------------------------------

void emptyHandler (int signal)
{
    printf ("I am empty\n");
}

//
// int sigSetInit (sigset_t* setPtr)
// {
//     int sigemptysetRet = sigemptyset(setPtr);
//     if (sigemptysetRet == -1)
//     {
//         perror ("Error with sigemptyset\n");
//         exit (0);
//     }
//
//     int sigaddsetRet = sigaddset (setPtr, SIGUSR1);
//     if (sigaddsetRet == -1)
//     {
//         perror ("Error with sigaddset\n");
//         exit (0);
//     }
//
//         sigaddsetRet = sigaddset (setPtr, SIGUSR2);
//     if (sigaddsetRet == -1)
//     {
//         perror ("Error with sigaddset\n");
//         exit (0);
//     }
//
//     return 0;
// }
//
//------------------------------------------------------------------------------
