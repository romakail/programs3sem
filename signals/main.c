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
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define BUFFER_LENGHT 128

#define PRINT(args...)      \
    printf(args);           \
    fflush (stdout);        \

int bit = 0;
int childContinue = 0;
pid_t parentPid = 0;

int  producer (pid_t consPid, const int fdFrom);
int  consumer (pid_t prodPid);

int  prodHandlersInit ();
int  consHandlersInit ();
int  blockUsrSignals  ();

char receiveByte (int prodPid);
int  throwByte   (int consPid, char byte);

void usr1HandlerCons      (int signal);
void usr2HandlerCons      (int signal);
void emptyHandler         (int signal);
void alarmHandlerProd     (int signal);
void childHandlerCons     (int signal);
void childContinueHandler (int signal);
//int  sigSetInit (sigset_t* setPtr);

//------------------------------------------------------------------------------

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        PRINT ("wrong number of arguments\n");
		return -1;
    }

    int fdFrom = open (argv[1], O_RDONLY);
    if (fdFrom == -1)
    {
        perror ("failed to open argument\n");
        exit (0);
    }

    parentPid = getpid();
    blockUsrSignals ();

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
    PRINT ("producer (child) pid = %d\n", getpid())
    // blockUsrSignals ();
    prodHandlersInit ();
	char buffer [BUFFER_LENGHT] = {};
	int readRet = 1;

	while (readRet > 0)
	{
		readRet = read(fdFrom, &buffer, BUFFER_LENGHT);
        for (int i = 0; i < readRet; i++)
        {
            throwByte (consPid, buffer[i]);
        }
	}
    throwByte (consPid, -1);
    // PRINT ("--------------------------------------------------- producer finished\n")
	return 0;
}

//------------------------------------------------------------------------------

int consumer (pid_t  prodPid)
{
    PRINT ("consumer (parent) pid = %d\n", getpid())
    // blockUsrSignals  ();
    consHandlersInit ();

    char letter = 0;

    letter = receiveByte (prodPid);
    if (letter != -1)
    {
        do
        {
            printf ("%c", letter);
            fflush (stdout);
            letter = receiveByte (prodPid);
        } while (letter != -1);
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

    action.sa_handler = childHandlerCons;
        sigActRet = sigaction (SIGCHLD, &action, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }

    // action.sa_handler = alarmHandler;
    // sigActRet = sigaction (SIGALRM, &action, 0);
    // if (sigActRet == -1)
    // {
    //     perror ("Error with sigaction\n");
    //     exit (0);
    // }

    return 0;
}

//------------------------------------------------------------------------------

int prodHandlersInit ()
{
    struct sigaction action = {};
    action.sa_handler = childContinueHandler;
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

    action.sa_handler = alarmHandlerProd;
    sigActRet = sigaction (SIGALRM, &action, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }

    return 0;
}

//------------------------------------------------------------------------------

int blockUsrSignals ()
{
    sigset_t mask;
    int ret = 0;
    ret = sigemptyset (&mask);
    if (ret == -1)
    {
        perror ("Error with sigemptyset\n");
        exit (0);
    }
    ret = sigaddset   (&mask, SIGUSR1);
    if (ret == -1)
    {
        perror ("Error with sigaddset\n");
        exit (0);
    }
    ret = sigaddset   (&mask, SIGUSR2);
    if (ret == -1)
    {
        perror ("Error with sigaddset\n");
        exit (0);
    }
    ret = sigprocmask (SIG_SETMASK, &mask, 0);
    if (ret == -1)
    {
        perror ("Error with sigprocmask\n");
        exit (0);
    }
    return 0;
}

//------------------------------------------------------------------------------

int throwByte (int consPid, char byte)
{
    // alarm (2);
    sigset_t emptyMask;
    sigemptyset (&emptyMask);
    // PRINT ("---------------------------BYTE = %c\n", byte);

    for (int i = 0; i < 8; i++)
    {
//--------------------------------critical 1 start
        if (((1 << (7-i)) & byte) == 0)
        {
            // PRINT ("bit = 0\n");
            kill (consPid, SIGUSR1);
        }
        else
        {
            // PRINT ("bit = 1\n");
            kill (consPid, SIGUSR2);
        }

        childContinue = 1;
        while (childContinue)
        {
            alarm (2);
            sigsuspend (&emptyMask);        // <----- critical 2: SIGALARM and SIGUSR fight
                                            // <----- father and child fidht for mask of came signals of child
//------------------------------critical 1 finish
            alarm (0);
        }
    }

	return 0;
}

//------------------------------------------------------------------------------

char receiveByte (int prodPid)
{
    // alarm (2);
    sigset_t emptyMask;
    sigemptyset (&emptyMask);

	char byte = 0;

    for (int i = 0; i < 8; i++)
    {
//----------------------------------critical 1 start
        sigsuspend (&emptyMask);
        byte = (byte << 1) | bit;
        kill (prodPid, SIGUSR2);
//----------------------------------crititcal 1 finish
    }

    // PRINT ("%c ", byte);
	return byte;
}

//------------------------------------------------------------------------------

void usr1HandlerCons (int signal)
{
    // PRINT ("thrown 0\n");
	bit = 0;
}

//------------------------------------------------------------------------------

void usr2HandlerCons (int signal)
{
    // PRINT ("thrown 1\n");
    bit = 1;
}

//------------------------------------------------------------------------------

void emptyHandler (int signal)
{
    // PRINT ("I am empty\n");
    // ready = 1;
    usleep (300);
    // sleep (3);
}

//------------------------------------------------------------------------------

void alarmHandlerProd (int signal)
{
    printf ("Alarm consumer triggered\n");
    printf ("dead's pid = %d\n", getppid());
    if (getppid() != parentPid)
    {
        printf ("My dad is dead=(\n");
        exit (0);
    }
}

//------------------------------------------------------------------------------

void childHandlerCons (int signal)
{
    printf ("Child(producer) died\n");
    exit (0);
}

//------------------------------------------------------------------------------

void childContinueHandler (int signal)
{
    // usleep (3000);
    childContinue = 0;
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
//
