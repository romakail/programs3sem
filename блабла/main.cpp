
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main ()
{
	int mypid = getpid ();
    char pidBuffer [10] = {};
    sprintf (pidBuffer, "%d", mypid);
	printf ("%s\n", pidBuffer);

    mkfifo(pidBuffer, 0644);

	return 0;
}
