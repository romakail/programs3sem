#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

struct msg
{
    long int mType;
    int payload;
};


int main(int argc, char** argv)
{
    //printf ("%s\n\n", argv[1]);

    if (argc != 2)
    {
        printf ("wrong number of argments\n");
        return 0;
    }
    else if (argc == 0)
    {
        printf ("something wrong with argv pointer\n");
        return 0;
    }
    errno = 0;
    char* endptr = 0;

    long int val = strtol (argv[1], &endptr, 10);


    if (errno == ERANGE)
    {
        printf ("overflow of long int\n");
        return 0;
    }
    else if (errno == EINVAL)
    {
        printf ("no digits seen\n");
        return 0;
    }
    else if (val < 0)
    {
        printf ("less then zero");
        return 0;
    }
    else if (*endptr != '\0')
    {
        printf ("I found not a digit\n");
        return 0;
    }


    int msgQueueId = msgget (IPC_PRIVATE, IPC_CREAT | 0644);
    struct msg message0;
    message0.mType = 1;

    //ssize_t msgrcvRet = msgrcv (msgQueueId, &message0, sizeof (int), 1, 0);
    //printf ("msgrcv returned %ld, errno = %d\n", msgrcvRet, errno);
    msgsnd (msgQueueId, &message0, sizeof(int), 0);

    if (msgQueueId == -1)
    {
        printf ("msgget returned error\n");
        return -1;
    }

	//printf ("msgQueueId = %d\n", msgQueueId);

    //printf ("val = %ld\n", val);

    pid_t pid = 1;

    //printf ("\n\n");
    //printf ("superparent = %d\n\n", getpid ());
    //printf ("val = %ld\n", val);

	//printf ("val = %ld\n", val);
    int procNumber = -1;
    for (int i = 1; i < val + 1; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            procNumber = i;
			break;
        }
	}


	//printf ("number = %d\n", procNumber);
	if (pid == 0)
	{
		struct msg receivedMessage;

		int rcvRet = msgrcv (msgQueueId, &receivedMessage, sizeof(int), procNumber, 0);
		if (rcvRet == -1)
			return -1;


		//printf ("n = %d\t", procNumber - 1);
		printf ("%d ", procNumber - 1);
		fflush(stdout);


		struct msg sentMessage;
		sentMessage.mType = procNumber + 1;

		int sndRet = msgsnd (msgQueueId, &sentMessage, sizeof(int), 0);
		if (sndRet == -1)
			return -1;

	}
	else
	{
		for (int i = 0; i < val; i++)
		{
            int waitRet = wait (0);
            //printf ("wait returned %d\n", waitRet);
		}
		int msgctlRet = msgctl (msgQueueId, IPC_RMID, NULL);
        //printf ("ret = %d killed\n", msgctlRet);
	}





    //printf ("%s\n", argv[1]);
    //printf ("%lg", strtoll(argv [1], 0, 10) )

    return 0;
}
