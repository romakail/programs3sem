#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#define CHILD_BUFFER_LENGHT 128
#define BLUE  "\x1B[34m"
#define NORM  "\x1B[0m"
#define RED   "\x1B[31m"

#define PRINT(args...) 		    \
	do 							\
	{							\
		printf(args);           \
		fflush (stdout);        \
	}while (0);


#define CHECK(what, message)										\
	do 																\
	{																\
		if (what == -1)												\
		{															\
			printf ("Error, line = %d", __LINE__);					\
			perror (message);										\
			exit(-1);												\
		}															\
	}while (0);

const int TRUE  = 1;
const int FALSE = 0;
const int RD = 0;
const int WR = 1;

struct pairInfo
{
	int childNumber;
	int fdFrw  [2];
	int fdBck [2];
	int parentBufLen;
	char* parentBuffer;
	// int childBufLen;
	// char* childBuffer;
	int allowRead;
	int nWrittenBytes;
	int nReadBytes;
	int eof;
};


struct pairInfo* initPairs (long int nChildren, int fdFrom, int fdTo);
int initFdSets (fd_set* readFdsPtr, fd_set* writeFdsPtr,struct pairInfo* pairs, int nChildren);
int freeMemory (struct pairInfo* pairs, int nChildren);

int child          (struct pairInfo* pairPtr);
int parent         (struct pairInfo* pairs, int nChildren);
int childCloseFds  (struct pairInfo* pairs, int nChildren, int childNum);
int parentCloseFds (struct pairInfo* pairs, int nChildren);

long int extractNum (const char* stringNum);
int bufferSize (int childNumber, int nChildren);
int power      (int x, int power);
int maxFdVal   (struct pairInfo* pairs, int nChildren);

int pairsDump (struct pairInfo* pairs, int nChildren);

int main (int argc, char** argv)
{
	if (argc != 3)
    {
        printf ("wrong number of arguments\n");
		return -1;
    }

    int fdFrom = open (argv[2], O_RDONLY);
    CHECK (fdFrom, "can't open the file")

	long int nChildren = extractNum(argv[1]);

	struct pairInfo* pairs = initPairs (nChildren, fdFrom, STDOUT_FILENO);

	pid_t pid  = 0;
	int procNumber = 0;
	int pipeRet = 0;

	for (int i = 0; i < nChildren ; i++)
    {

        pid = fork();
        if (pid == 0)   // child
        {
            procNumber = i;
			break;
        }
	}

	if (pid == 0)  //child
	{
		childCloseFds (pairs, nChildren, procNumber);
		child  (&(pairs[procNumber]));
	}
	else
	{
		parentCloseFds (pairs, nChildren);
		parent (pairs, nChildren);
	}
	freeMemory (pairs, nChildren);

	return 0;
}

//------------------------------------------------------------------------------

int parent (struct pairInfo* pairs, int nChildren)
{
	// PRINT ("I am parent I am ready to write\n")
	fd_set  readFds;
	fd_set writeFds;

	int   readRet = 0;
	int  writeRet = 0;

	// int selectRetRead  = 0;
	// int selectRetWrite = 0;
	int selectRet = 0;

	int nAvailableFD = 0;

	int maxFd = maxFdVal (pairs, nChildren) + 1;

	while (maxFd)
	{
		initFdSets (&readFds, &writeFds, pairs, nChildren);

		selectRet = select(maxFd, &readFds, &writeFds, 0, 0);
		CHECK (selectRet , "Error with select\n");

		// selectRetWrite = select(maxFd, 0, &writeFds, 0, 0);
		// CHECK (selectRetWrite, "Error with select\n");

		// PRINT (BLUE "selectReturned = %d\n" NORM, selectRet);

		for (int i = 0; i < nChildren - 1; i++)
		{
			if (FD_ISSET(pairs[i].fdBck[RD], &readFds) && (pairs[i].allowRead == TRUE))
			{
				pairs[i].allowRead = FALSE;
				pairs[i].nWrittenBytes = 0;
				readRet = read (pairs[i].fdBck[RD], pairs[i].parentBuffer, pairs[i].parentBufLen);
				CHECK (readRet, "Error with read in parent\n");
				if (readRet == 0)
				{
					close (pairs[i]  .fdBck[RD]);
					close (pairs[i+1].fdFrw[WR]);
					pairs[i]  .fdBck[RD] = -1;
					pairs[i+1].fdFrw[WR] = -1;
				}
				pairs[i].nReadBytes = readRet;
			}
			if (FD_ISSET(pairs[i+1].fdFrw[WR], &writeFds))
			{
				writeRet = write (pairs[i+1].fdFrw[WR], pairs[i].parentBuffer + pairs[i].nWrittenBytes, pairs[i].nReadBytes - pairs[i].nWrittenBytes);
				CHECK (writeRet, "Erroe with write in parent\n");
				pairs[i].nWrittenBytes += writeRet;
			}

			if (pairs[i].nReadBytes == pairs[i].nWrittenBytes)
			{
				pairs[i].allowRead = TRUE;
			}
		}
		maxFd = maxFdVal (pairs, nChildren) + 1;
	}

	return 0;
}

//------------------------------------------------------------------------------

int child (struct pairInfo* pairPtr)
{
	char buffer [CHILD_BUFFER_LENGHT] = {};
	int readRet  = 1;
	int writeRet = 0;

	while (readRet != 0)
	{
		readRet  = read (pairPtr->fdFrw[RD], buffer, CHILD_BUFFER_LENGHT);
		CHECK (readRet , "problem with read\n")

		sleep(1);

		writeRet = write(pairPtr->fdBck[WR], buffer, readRet);
		CHECK (writeRet, "problem with write\n")
	}

	close (pairPtr->fdFrw[RD]);
	close (pairPtr->fdBck[WR]);
	// int selectRetRead  = 0;
	// int selectRetRead  = 0;
	// int selectRetRead  = 0;
	// int selectRet
	pairPtr->fdFrw[RD] = -1;
	pairPtr->fdBck[WR] = -1;

	return 0;
}

//------------------------------------------------------------------------------

int childCloseFds (struct pairInfo* pairs, int nChildren, int childNum)
{
	int closeRet = 0;

	for (int i = 0; i < nChildren; i++)
	{

		if (i == childNum)
		{
			if ((pairs[i].fdFrw[WR] != -1) && (pairs[i].fdFrw[WR] != 1))
			{
				closeRet = close (pairs[i].fdFrw[WR]);
				CHECK (closeRet, "Error with close\n");
			}
			if (pairs[i].fdBck[RD] != -1)
			{
				closeRet = close (pairs[i].fdBck[RD]);
				CHECK (closeRet, "Error with close\n");
			}
		}
		else
		{
			if (pairs[i].fdFrw[WR] != -1)
			{
				closeRet = close (pairs[i].fdFrw[WR]);
				CHECK (closeRet, "Error with close\n");
			}
			if (pairs[i].fdFrw[RD] != -1)
			{
				closeRet = close (pairs[i].fdFrw[RD]);
				CHECK (closeRet, "Error with close\n");
			}
			if ((pairs[i].fdBck[WR] != -1) && (pairs[i].fdBck[WR] != 1))
			{
				closeRet = close (pairs[i].fdBck[WR]);
				CHECK (closeRet, "Error with close\n");
			}
			if (pairs[i].fdBck[RD] != -1)
			{
				closeRet = close (pairs[i].fdBck[RD]);
				CHECK (closeRet, "Error with close\n");
			}
		}
	}
	return 0;
}

//------------------------------------------------------------------------------

int parentCloseFds (struct pairInfo* pairs, int nChildren)
{
	int closeRet = 0;

	for (int i = 0; i < nChildren; i++)
	{
		closeRet = close (pairs[i].fdFrw[RD]);
		CHECK (closeRet, "Error with close\n");
		if (pairs[i].fdBck[WR] != 1)
		{
			closeRet = close (pairs[i].fdBck[WR]);
			CHECK (closeRet, "Error with close\n");
		}
	}
	return 0;
}

//------------------------------------------------------------------------------

int initFdSets (fd_set* readFdsPtr, fd_set* writeFdsPtr, struct pairInfo* pairs, int nChildren)
{
	FD_ZERO ( readFdsPtr);
	FD_ZERO (writeFdsPtr);

	for (int i = 0; i < nChildren - 1; i++)
	{
		if (pairs[i]  .fdBck[RD] != -1)
			FD_SET (pairs[i]  .fdBck[RD],  readFdsPtr);
		if (pairs[i+1].fdFrw[WR] != -1)
			FD_SET (pairs[i+1].fdFrw[WR], writeFdsPtr);
	}
	return 0;
}

//------------------------------------------------------------------------------

struct pairInfo* initPairs (long int nChildren, int fdFrom, int fdTo)
{
	struct pairInfo* pairs = (struct pairInfo*) calloc (nChildren, sizeof (*pairs));

	for (int i = 0; i < nChildren - 1; i++)
	{
		pairs[i].parentBufLen = bufferSize(i, nChildren);
		pairs[i].parentBuffer = (char*) calloc (pairs[i].parentBufLen, sizeof(char));
		if (pairs[i].parentBuffer == 0)
		{
			printf ("i = %d\n", i);
			perror ("Error with calloc\n");
			exit (-1);
		}
		pairs[i].allowRead = 1;
	}

	pairs[nChildren - 1].parentBufLen = -1;
	pairs[nChildren - 1].parentBuffer = 0;

	for (int i = 0; i < nChildren; i++)
	{
		pairs[i].childNumber = i;
	}

	// pipes & file descriptors

	if (nChildren == 1)
	{
		pairs [0].fdFrw[WR] = -1;
		pairs [0].fdFrw[RD] = fdFrom;

		pairs [0].fdBck[WR] = fdTo;
		pairs [0].fdBck[RD] = -1;
	}
	else if (nChildren > 1)
	{
		pairs [0].fdFrw[WR] = -1;
		pairs [0].fdFrw[RD] = fdFrom;

		int ret = pipe (pairs[0].fdBck);
		CHECK (ret, "problem with pipe\n");

		ret = fcntl (pairs[0].fdBck[RD], F_SETFL, O_RDONLY | O_NONBLOCK);
		CHECK (ret, "problem with fcntl\n")

		for (int i = 1; i < nChildren - 1; i++)
		{
			ret = pipe(pairs[i].fdFrw);
			CHECK (ret, "problem with pipe\n")
			ret = pipe(pairs[i].fdBck);
			CHECK (ret, "problem with pipe\n")

			ret = fcntl (pairs[i].fdFrw[WR], F_SETFL, O_WRONLY | O_NONBLOCK);
			CHECK (ret, "problem with fcntl\n")
			ret = fcntl (pairs[i].fdBck[RD], F_SETFL, O_RDONLY | O_NONBLOCK);
			CHECK (ret, "problem with fcntl\n")

		}

		ret = pipe (pairs[nChildren - 1].fdFrw);
		CHECK (ret, "problem with pipe\n");

		ret = fcntl (pairs[nChildren - 1].fdFrw[WR], F_SETFL, O_WRONLY | O_NONBLOCK);
		CHECK (ret, "problem with fcntl\n")

		pairs [nChildren - 1].fdBck[WR] = fdTo;
		pairs [nChildren - 1].fdBck[RD] = -1;
	}
	else
	{
		printf ("nChildren is <= 0\n");
		exit (-1);
	}

	return pairs;
}

//------------------------------------------------------------------------------

int freeMemory (struct pairInfo* pairs, int nChildren)
{
	for (int i = 0; i < nChildren - 1; i++)
		free (pairs[i].parentBuffer);
	free(pairs);

	return 0;
}

//------------------------------------------------------------------------------

int pairsDump (struct pairInfo* pairs, int nChildren)
{
	for (int i = 0; i < nChildren; i++)
	{
		PRINT ("---childNo %d---\n", pairs[i].childNumber);
		PRINT ("Frw[WR] = %d, Frw[RD] = %d, ", pairs[i].fdFrw[WR], pairs[i].fdFrw[RD]);
		PRINT ("Bck[WR] = %d, Bck[RD] = %d\n", pairs[i].fdBck[WR], pairs[i].fdBck[RD]);
		PRINT ("bufferPtr = %p, bufferLen = %d\n", pairs[i].parentBuffer, pairs[i].parentBufLen);
	}

	return 0;
}

//------------------------------------------------------------------------------

int maxFdVal (struct pairInfo* pairs, int nChildren)
{
	int maxFd = -1;
	for (int i = 0; i < nChildren; i++)
	{
		if (pairs[i].fdFrw[WR] > maxFd)
			maxFd = pairs[i].fdFrw[WR];
		// if (pairs[i].fdFrw[RD] > maxFd)
		// 	maxFd = pairs[i].fdFrw[RD];
		// if (pairs[i].fdBck[WR] > maxFd)
		// 	maxFd = pairs[i].fdBck[WR];
		if (pairs[i].fdBck[RD] > maxFd)
			maxFd = pairs[i].fdBck[RD];
	}
	return maxFd;
}

//------------------------------------------------------------------------------

int bufferSize (int childNumber, int nChildren)
{
	int a = 1024 * power(3, nChildren - childNumber);
	int retVal = (a < 131072) ? a : 131072;
	return	retVal;
}

//------------------------------------------------------------------------------

int power (int x, int power)
{
	long int ret = 1;
	for (int i = 0; i < ((power < 9) ? power : 9); i++)
		ret *= x;
	return ret;
}

//------------------------------------------------------------------------------

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

//---vse ya konchilsya
