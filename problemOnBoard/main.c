
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


// 666 (10) == 1 010 011 010 (2)
// user  : r = 0
// 	       w = 1
// 		   e = 0
// group : r = 0
// 	       w = 1
// 		   e = 1
// all   : r = 0
// 		   w = 1
// 		   e = 0

int main (int argc, char** argv)
{
	char buf [3] = {};

	mkfifo ("fifo", 666);
	int fd = open ("fifo", O_RDWR);
	int writeRet = write (fd, "aaa", 3);
	fd = open ("fifo", O_RDONLY | O_NONBLOCK);
	write (1, buf, read (fd, buf, 3));

	// if (05 == 5)
	// 	printf ("equal\n");
	// else
	// 	printf ("not equal\n");


	// printf ("1 = %d\n", 0b10);
	// printf ("2 = %d\n", 010);
	// printf ("3 = %d\n", 10);
	// printf ("4 = %d\n", 0x10);

	return 0;
}
