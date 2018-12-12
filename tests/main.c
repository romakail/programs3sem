
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
#include <sys/time.h>
#include <errno.h>


long int extractNum (const char* stringNum);

int isSameFile (int fd1, int fd2);

int main (int argc, char** argv)
{
	int ret = 1;
	int power = 30;
	int x = 3;
	for (int i = 0; i < power; i++)
	{
		ret *= x;
		printf ("i = %d, ret = %d\n", i, ret);
	}
}

int isSameFile (int fd1, int fd2)
{
	struct stat stat1;
	struct stat stat2;

	int ret = fstat(fd1, &stat1);
	if (ret == -1)
	{
		perror ("Fstat dont work\n");
		return 0;
	}

		ret = fstat(fd2, &stat2);
	if (ret == -1)
	{
		perror ("Fstat dont work\n");
		return 0;
	}

	if ((stat1.st_ino == stat2.st_ino) && (stat1.st_dev == stat2.st_dev))
		return 1;
	else
		return 0;
}


long int extractNum (const char* stringNum)
{
	char* endptr = 0;
	long int val = strtol (stringNum, &endptr, 10);

	if (errno == ERANGE)
	{
		perror ("overflow of long int\n");
		exit (0);
	}
	else if (errno == EINVAL)
	{
		perror ("no digits seen\n");
		exit (0);
	}
	else if (val < 0)
	{
		perror ("less then zero");
		exit (0);
	}
	else if (*endptr != '\0')
	{
		perror ("I found not a digit\n");
		exit (0);
	}

	return val;
}



// int main ()
// {
// 	sigset_t set;
// 	sigSetInit (&set);
//
// 	struct sigaction act = {};
// 	act.sa_handler = handler1;
//     int sigActRet = sigaction (SIGUSR1, &act, 0);
//     if (sigActRet == -1)
//     {
//         perror ("Error with sigaction\n");
//         exit (0);
//     }
//
// 	act.sa_handler = handler2;
// 	    sigActRet = sigaction (SIGUSR2, &act, 0);
//     if (sigActRet == -1)
//     {
//         perror ("Error with sigaction\n");
//         exit (0);
//     }
//
// 	sigsuspend(&set);
// 	printf ("This is the end for you my friend\n");
// 	return 0;
// }
//
//
// void handler1 (int signal)
// {
// 	printf ("che1\n");
// }
//
// void handler2 (int signal)
// {
// 	printf ("che2\n");
// }
//
// int sigSetInit (sigset_t* setPtr)
// {
//     int sigemptysetRet = sigemptyset(setPtr);
//     if (sigemptysetRet == -1)
//     {
//         perror ("Error with sigemptyset\n");
//         exit (0);
//     }

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
//
//     return 0;
// }
