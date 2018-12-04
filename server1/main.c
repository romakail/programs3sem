#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#define CHILD_BUFFER_LENGHT 1024

const int RD = 0;
const int WR = 1;

struct pairInfo
{
	int fdFrw  [2];
	int fdBack [2];
	int parrentBufLen;
	char* parrentBuffer;
	int childBufLen
	char* childBuffer;

};

long int extractNum (const char* stringNum);

int main (int argc, char** argv)
{
	if (argc != 3)
    {
        PRINT ("wrong number of arguments\n");
		return -1;
    }

    int fdFrom = open (argv[2], O_RDONLY);
    if (fdFrom == -1)
    {
        perror ("failed to open file IN\n");
        exit (0);
    }

	struct pairInfo* pairs = NULL;

	long int nChildren = extractNum(argv[1]);

	pid_t pid  = 0;
	int procNumber = 0;
	int pipeRet = 0;

	for (int i = 0; i < nChildren ; i++)
    {
		pipeRet = pipe (pairs[i].fdFrw);
		if (pipeRet = -1)
		{
			perror ("Error with pipe");
			exit (0);
		}
		pipeRet = pipe (pairs[i].fdBack);
		if (pipeRet = -1)
		{
			perror ("Error with pipe");
			exit (0);
		}

        pid = fork();
        if (pid == 0)   // child
        {
            procNumber = i;
			break;
        }
		else
		{

		}
	}

	if (pid == 0)
	{

	}

	return 0;
}

//------------------------------------------------------------------------------

int initPairs (long int nChildren, struct pairInfo* pairs, )
{
	pairs = (struct piarInfo*) calloc (nChildren, sizeof (pairs));

	for (int i = 0; )

}

//------------------------------------------------------------------------------

long int extractNum (const char* stringNum)
{
	char* endptr = 0;
	long int val = strtol (argv[1], &endptr, 10);

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
