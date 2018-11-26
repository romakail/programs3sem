
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int sigSetInit (sigset_t* setPtr);
void handler1 (int signal);
void handler2 (int signal);

int main ()
{
	sigset_t set;
	sigSetInit (&set);

	struct sigaction act = {};
	act.sa_handler = handler1;
    int sigActRet = sigaction (SIGUSR1, &act, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }

	act.sa_handler = handler2;
	    sigActRet = sigaction (SIGUSR2, &act, 0);
    if (sigActRet == -1)
    {
        perror ("Error with sigaction\n");
        exit (0);
    }

	sigsuspend(&set);
	printf ("This is the end for you my friend\n");
	return 0;
}


void handler1 (int signal)
{
	printf ("che1\n");
}

void handler2 (int signal)
{
	printf ("che2\n");
}

int sigSetInit (sigset_t* setPtr)
{
    int sigemptysetRet = sigemptyset(setPtr);
    if (sigemptysetRet == -1)
    {
        perror ("Error with sigemptyset\n");
        exit (0);
    }

    // int sigaddsetRet = sigaddset (setPtr, SIGUSR1);
    // if (sigaddsetRet == -1)
    // {
    //     perror ("Error with sigaddset\n");
    //     exit (0);
    // }
	//
    //     sigaddsetRet = sigaddset (setPtr, SIGUSR2);
    // if (sigaddsetRet == -1)
    // {
    //     perror ("Error with sigaddset\n");
    //     exit (0);
    // }

    return 0;
}
