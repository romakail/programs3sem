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

#define PRINT(args...) 	//	    \
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
};

long int extractNum (const char* stringNum);
int bufferSize (int childNumber, int nChildren);
int power (int x, int power);

struct pairInfo* initPairs (long int nChildren, int fdFrom, int fdTo);
int child  (struct pairInfo* pairPtr);
int parent (struct pairInfo* pairs, int nChildren);
int initFdSets (fd_set* readFdsPtr, fd_set* writeFdsPtr,struct pairInfo* pairs, int nChildren);

int maxFdVal (struct pairInfo* pairs, int nChildren);

int pairsDump (struct pairInfo* pairs, int nChildren);

int childCloseFds (struct pairInfo* pairs, int nChildren, int childNum);
int parentCloseFds (struct pairInfo* pairs, int nChildren);

int main (int argc, char** argv)
{
	if (argc != 3)
    {
        printf ("wrong number of arguments\n");
		return -1;
    }

    int fdFrom = open (argv[2], O_RDONLY);
	PRINT ("fdFrom = %d\n", fdFrom);
    CHECK (fdFrom, "can't open the file")


	long int nChildren = extractNum(argv[1]);

	struct pairInfo* pairs = initPairs (nChildren, fdFrom, STDOUT_FILENO);
	pairsDump (pairs, nChildren);

	pid_t pid  = 0;
	int procNumber = 0;
	int pipeRet = 0;

	for (int i = 0; i < nChildren ; i++)
    {
		// pipeRet = pipe (pairs[i].fdFrw);
		// CHECK (pipeRet, "Error with pipe\n")
		//
		// pipeRet = pipe (pairs[i].fdBck);
		// CHECK (pipeRet, "Error with pipe\n")

        pid = fork();
        if (pid == 0)   // child
        {
            procNumber = i;
			break;
        }
	}

	if (pid == 0)  //child
	{
		// PRINT ("my pid = %d\n", getpid());
		// PRINT ("childPointer [%d] = %p\n", procNumber, &(pairs[procNumber]));
		childCloseFds (pairs, nChildren, procNumber);
		// PRINT ("procNumber = %d\n", procNumber)
		child  (&(pairs[procNumber]));
	}
	else
	{
		// PRINT ("myPid = %d\n", getpid());
		// PRINT ("parentPointer = %p\n", pairs);
		// PRINT ("nChildren = %ld\n", nChildren);
		parentCloseFds (pairs, nChildren);
		parent (pairs, nChildren);
	}

	return 0;
}

//------------------------------------------------------------------------------

int parent (struct pairInfo* pairs, int nChildren)
{
	PRINT ("I am a parent and I have an ability to write\n")
	fd_set  readFds;
	fd_set writeFds;

	int   readRet = 0;
	int  writeRet = 0;
	int selectRet = 0;

	int maxFd = maxFdVal (pairs, nChildren) + 1;

	while (1)
	{
		initFdSets (&readFds, &writeFds, pairs, nChildren);

		selectRet = select(maxFd, &readFds, &writeFds, 0, 0);
		CHECK (selectRet, "Error with select\n")

		for (int i = 0; i < nChildren - 1; i++)
		{
			if (FD_ISSET(pairs[i].fdBck[RD], &readFds) && (pairs[i].allowRead == TRUE))
			{
				pairs[i].allowRead = FALSE;
				pairs[i].nWrittenBytes = 0;
				readRet = read (pairs[i].fdBck[RD], pairs[i].parentBuffer, pairs[i].parentBufLen);
				CHECK (readRet, "Error with read in parent\n")
				pairs[i].nReadBytes = readRet;
			}
			if (FD_ISSET(pairs[i+1].fdFrw[WR], &writeFds))
			{
				writeRet = write (pairs[i+1].fdFrw[WR], pairs[i].parentBuffer + pairs[i].nWrittenBytes, pairs[i].nReadBytes - pairs[i].nWrittenBytes);
				CHECK (writeRet, "Erroe with write in parent\n")
				pairs[i].nWrittenBytes += writeRet;
			}

			if (pairs[i].nReadBytes == pairs[i].nWrittenBytes)
			{
				pairs[i].allowRead = TRUE;
			}
		}
	}

	return 0;
}

//------------------------------------------------------------------------------

int child (struct pairInfo* pairPtr)
{
	PRINT ("Child %d, poehali fdFrw[RD] = %d fdBck[WR] = %d, myPid = %d\n", pairPtr->childNumber, pairPtr->fdFrw[RD], pairPtr->fdBck[WR], getpid())

	char buffer [CHILD_BUFFER_LENGHT] = {};
	// printf ("===============bufferPtr = %p\n" , buffer);
	int readRet  = 1;
	int writeRet = 0;

	while (readRet != 0)
	{
		readRet  = read (pairPtr->fdFrw[RD], buffer, CHILD_BUFFER_LENGHT);
		PRINT ("                                                        child %d, read    %d bytes from %d\n", pairPtr->childNumber, readRet, pairPtr->fdFrw[RD]);
		CHECK (readRet , "problem with read\n")

		writeRet = write(pairPtr->fdBck[WR], buffer, readRet);
		PRINT ("                                                        child %d, written %d bytes into %d\n", pairPtr->childNumber, writeRet, pairPtr->fdBck[WR]);
		CHECK (writeRet, "problem with write\n")
	}

	return 0;
}

//------------------------------------------------------------------------------

int childCloseFds (struct pairInfo* pairs, int nChildren, int childNum)
{
	// PRINT ("child %d, started closing fds; pair = %p, nChildren = %d\n", childNum, pairs, nChildren);
	int closeRet = 0;

	for (int i = 0; i < nChildren; i++)
	{

		if (i == childNum)
		{
			if ((pairs[i].fdFrw[WR] != -1) && (pairs[i].fdFrw[WR] != 1))
			{
				closeRet = close (pairs[i].fdFrw[WR]);
				// PRINT ("Child %d, closing %d, ret = %d\n", childNum, pairs[i].fdFrw[WR], closeRet)
				CHECK (closeRet, "Error with close\n");
			}
			if (pairs[i].fdBck[RD] != -1)
			{
				closeRet = close (pairs[i].fdBck[RD]);
				// PRINT ("Child %d, closing %d, ret = %d\n", childNum, pairs[i].fdBck[RD], closeRet)
				CHECK (closeRet, "Error with close\n");
			}
		}
		else
		{
			if (pairs[i].fdFrw[WR] != -1)
			{
				closeRet = close (pairs[i].fdFrw[WR]);
				// PRINT ("Child %d, closing %d, ret = %d\n", childNum, pairs[i].fdFrw[WR], closeRet)
				CHECK (closeRet, "Error with close\n");
			}
			if (pairs[i].fdFrw[RD] != -1)
			{
				closeRet = close (pairs[i].fdFrw[RD]);
				// PRINT ("Child %d, closing %d, ret = %d\n", childNum, pairs[i].fdFrw[RD], closeRet)
				CHECK (closeRet, "Error with close\n");
			}
			if ((pairs[i].fdBck[WR] != -1) && (pairs[i].fdBck[WR] != 1))
			{
				closeRet = close (pairs[i].fdBck[WR]);
				// PRINT ("Child %d, closing %d, ret = %d\n", childNum, pairs[i].fdBck[WR], closeRet)
				CHECK (closeRet, "Error with close\n");
			}
			if (pairs[i].fdBck[RD] != -1)
			{
				closeRet = close (pairs[i].fdBck[RD]);
				// PRINT ("Child %d, closing %d, ret = %d\n", childNum, pairs[i].fdBck[RD], closeRet)
				CHECK (closeRet, "Error with close\n");
			}
		}
	}
	// PRINT ("Child %d, closed unnecessary descriptors\n", childNum);
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
	PRINT ("Parent, closed unnecessary descriptors\n");
	return 0;
}

//------------------------------------------------------------------------------

int initFdSets (fd_set* readFdsPtr, fd_set* writeFdsPtr, struct pairInfo* pairs, int nChildren)
{
	FD_ZERO ( readFdsPtr);
	FD_ZERO (writeFdsPtr);

	for (int i = 0; i < nChildren - 1; i++)
	{
		FD_SET (pairs[i]  .fdBck[RD],  readFdsPtr);
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

	pairs [0].fdFrw[WR] = -1;
	pairs [0].fdFrw[RD] = fdFrom;

	int ret = pipe (pairs[0].fdBck);
	PRINT ("pair 0 fdBck[WR] = %d, fdBck[RD] = %d\n", pairs[0].fdBck[WR], pairs[0].fdBck[RD])
	CHECK (ret, "problem with pipe\n");

	for (int i = 1; i < nChildren - 1; i++)
	{
		ret = pipe(pairs[i].fdFrw);
		PRINT ("pair %d fdFrw[WR] = %d, fdFrw[RD] = %d\n", i, pairs[i].fdFrw[WR], pairs[i].fdFrw[RD])
		CHECK (ret, "problem with pipe\n")
		ret = pipe(pairs[i].fdBck);
		PRINT ("pair %d fdBck[WR] = %d, fdBck[RD] = %d\n", i, pairs[i].fdBck[WR], pairs[i].fdBck[RD])
		CHECK (ret, "problem with pipe\n")

		// ret = fcntl (pairs[i].fdFrw[RD], SET_FL, O_RDONLY);
		// CHECK (ret, "problem with fcntl\n")
		// ret = fcntl (pairs[i].fdBck[WR], SET_FL, O_WRONLY);
		// CHECK (ret, "problem with fcntl\n")

		ret = fcntl (pairs[i].fdFrw[WR], F_SETFL, O_WRONLY | O_NONBLOCK);
		CHECK (ret, "problem with fcntl\n")
		ret = fcntl (pairs[i].fdBck[RD], F_SETFL, O_RDONLY | O_NONBLOCK);
		CHECK (ret, "problem with fcntl\n")

		// ret = fcntl (pairs[i].fdFrw[RD], F_SETFL, O_RDONLY | O_NONBLOCK);
		// CHECK (ret, "problem with fcntl\n")
		// ret = fcntl (pairs[i].fdBck[WR], F_SETFL, O_WRONLY | O_NONBLOCK);
		// CHECK (ret, "problem with fcntl\n")

	}

	ret = pipe (pairs[nChildren - 1].fdFrw);
	PRINT ("pair nChildren - 1 fdFrw[WR] = %d, fdFrw[RD] = %d\n", pairs[nChildren - 1].fdFrw[WR], pairs[nChildren - 1].fdFrw[RD])
	CHECK (ret, "problem with pipe\n");

	pairs [nChildren - 1].fdBck[WR] = fdTo;
	pairs [nChildren - 1].fdBck[RD] = -1;

	return pairs;
}

//------------------------------------------------------------------------------

// struct pairInfo
// {
// 	int childNumber;
// 	int fdFrw  [2];
// 	int fdBck [2];
// 	int parentBufLen;
// 	char* parentBuffer;
// 	int childBufLen;
// 	char* childBuffer;
//
// };

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
		if (pairs[i].fdFrw[RD] > maxFd)
			maxFd = pairs[i].fdFrw[RD];
		if (pairs[i].fdBck[WR] > maxFd)
			maxFd = pairs[i].fdBck[WR];
		if (pairs[i].fdBck[RD] > maxFd)
			maxFd = pairs[i].fdBck[RD];
	}
	return maxFd;
}

//------------------------------------------------------------------------------

int bufferSize (int childNumber, int nChildren)
{
	// PRINT ("power = %d", power(3, nChildren - childNumber));
	int a = 1024 * power(3, nChildren - childNumber);
	int retVal = (a < 131072) ? a : 131072;
	// PRINT ("bufferSize = %d\n", retVal);
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
