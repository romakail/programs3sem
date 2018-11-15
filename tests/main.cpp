
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>


int main ()
{

	pid_t pid = 0;
	pid = fork ();
	if (pid == 0)
		sleep(3);
    else
    {
		int a = wait(0);
        printf ("printf wait1 returned %d\n", a);
        int b = wait(0);
        printf ("printf wait1 returned %d\n", b);
	}

	return 0;
}
