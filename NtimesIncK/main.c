#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


void* incKtimes (void* K);

int Z = 0;

int main(int argc, char** argv)
{
    //printf ("%s\n\n", argv[1]);

    if (argc != 3)
    {
        printf ("wrong number of argments\n");
        return 0;
    }
    else if (argc == 0)
    {
        printf ("something wrong with argv pointer\n");
        return 0;
    }


    //long val = 0;
    errno = 0;
    char* endptr = 0;

    int N = strtol (argv[1], &endptr, 10);
    long int K = strtol (argv[2], &endptr, 10);

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
    else if (((N < 0) || (K < 0)))
    {
        printf ("one of args is less then zero");
        return 0;
    }
    else if (*endptr != '\0')
    {
        printf ("I found not a digit\n");
        return 0;
    }

    printf ("N = %d, K = %ld\n\n", N, K);

    void* (*run) (void*);
    run = incKtimes;

    //pthread_t thread;
    pthread_t* threads = (pthread_t*) calloc (N, sizeof(pthread_t));
    //static_cast <pthread_t*>(calloc ());

    for (int i = 0; i < N; i++)
    {
        //printf ("i = %d\n", i);
        pthread_create(&(threads[i]), 0, run, (void*)K);
    }



    for (int i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }




    //printf ("%s\n", argv[1]);
    //printf ("%lg", strtoll(argv [1], 0, 10) )


    //sleep (2);
    printf ("Z = %d\n\n", Z);
    return 0;
}


void* incKtimes (void* K)
{
    //printf ()
    //printf ("Z before = %d\n", Z);
    for (int i = 0; i < (long int)K; i++)
        Z++;

    //printf ("Z after = %d\n", Z);
    return 0;
}
