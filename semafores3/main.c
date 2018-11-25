#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

#define SHARED_MEMORY_KEY 1837
#define SIZE_OF_SHARED_BUFFER 128
#define SEMAFORES_KEY 1818


#define PRINT(args...)      \
    printf(args);           \
    fflush (stdout);        \


#define sem_do(semaphore, operation, flag)      \
    sops[0].sem_num = semaphore;                \
    sops[0].sem_op  = operation;                \
    sops[0].sem_flg = flag;                     \
    if (semop(semId, sops, 1) == -1)            \
    {                                           \
        printf ("line  = %d\n", __LINE__);      \
        perror ("Error with semop\n");          \
        return -1;                              \
    }


#define sem_do2(semaphore1, operation1, flag1, semaphore2, operation2, flag2)   \
    sops[0].sem_num = semaphore1;               \
    sops[0].sem_op  = operation1;               \
    sops[0].sem_flg = flag1;                    \
                                                \
    sops[1].sem_num = semaphore2;               \
    sops[1].sem_op  = operation2;               \
    sops[1].sem_flg = flag2;                    \
                                                \
    if (semop(semId, sops, 2) == -1)            \
    {                                           \
        printf ("line  = %d\n", __LINE__);      \
        perror ("Error with semop\n");          \
        return -1;                              \
    }

#define sem_do3(semaphore1, operation1, flag1, semaphore2, operation2, flag2, semaphore3, operation3, flag3)   \
    sops[0].sem_num = semaphore1;               \
    sops[0].sem_op  = operation1;               \
    sops[0].sem_flg = flag1;                    \
                                                \
    sops[1].sem_num = semaphore2;               \
    sops[1].sem_op  = operation2;               \
    sops[1].sem_flg = flag2;                    \
                                                \
    sops[2].sem_num = semaphore3;               \
    sops[2].sem_op  = operation3;               \
    sops[2].sem_flg = flag3;                    \
                                                \
    semopRet = semop(semId, sops, 3);           \
    printf ("semop = %d\n", semopRet);          \
    if (semopRet == -1)                         \
    {                                           \
        printf ("line  = %d\n", __LINE__);      \
        perror ("Error with semop\n");          \
        return -1;                              \
    }


#define sem_do4(semaphore1, operation1, flag1, semaphore2, operation2, flag2, semaphore3, operation3, flag3, semaphore4, operation4, flag4)     \
    sops[0].sem_num = semaphore1;               \
    sops[0].sem_op  = operation1;               \
    sops[0].sem_flg = flag1;                    \
                                                \
    sops[1].sem_num = semaphore2;               \
    sops[1].sem_op  = operation2;               \
    sops[1].sem_flg = flag2;                    \
                                                \
    sops[2].sem_num = semaphore3;               \
    sops[2].sem_op  = operation3;               \
    sops[2].sem_flg = flag3;                    \
                                                \
    sops[3].sem_num = semaphore4;               \
    sops[3].sem_op  = operation4;               \
    sops[3].sem_flg = flag4;                    \
                                                \
    if (semop(semId, sops, 4) == -1)            \
    {                                           \
        printf ("line  = %d\n", __LINE__);      \
        perror ("Error with semop\n");          \
        return -1;                              \
    }


enum semafore_names
{
    INITIALIZATOR,
    NO_STR_FOR_STR,
    NO_FOL_FOR_STR,
    NO_STR_FOR_FOL,
    NO_FOL_FOR_FOL,
    MUT_EX,
    FULL,
    EMPTY,
    NSEMAFORES
};

struct package
{
    int size;
    char buffer [SIZE_OF_SHARED_BUFFER];
};


int modeDetection (char* mode);
int initializeSemafores (int semId);
int dumpSemafores (int semId);
int follower (void* shMemPtr, int semId);
int streamer (void* shMemPtr, int semId, char* fileFromName);

int main (int argc, char** argv)
{
    if ((argc > 3) || (argc == 1))
    {
        printf ("wrong number of arguments\n");
        return 0;
    }
    else if (argc == 0)
    {
        printf ("something wrong with argv pointer\n");
        return 0;
    }

    int mode = modeDetection (argv[1]);
    if (mode == -1)
        return -1;
    //printf ("mode = %d\n", mode);

    int semId = semget (SEMAFORES_KEY, NSEMAFORES, IPC_CREAT | 0666);
    PRINT ("semId = %d\n", semId)
    if (semId == -1)
    {
        perror ("error with semget\n");
        return -1;
    }

    int shmId = shmget (SHARED_MEMORY_KEY, sizeof(struct package), IPC_CREAT | 0666);
    PRINT ("shmId = %d\n", shmId)
    if (shmId == -1)
    {
        perror ("error with shmget\n");
        return -1;
    }

    void* shMemPtr = shmat (shmId, NULL, 0);
    PRINT ("shMemPtr = %p\n", shMemPtr)
    if (shMemPtr == (void*)-1)
    {
        perror ("error with shmat\n");
        return -1;
    }

    if (mode == 0) // streamer
    {
        if (argc == 2)
        {
            printf ("no file\n");
            return 0;
        }

        streamer(shMemPtr, semId, argv[2]);

    }
    else if (mode == 1) // follower
    {
        if (argc == 3)
        {
            printf ("I dont need any more arguments in this mode\n");
            return 0;
        }

        follower (shMemPtr, semId);
    }
    else
    {
        printf ("some mistake occured\n");
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------
//                   mode = 0
int streamer (void* shMemPtr, int semId, char* fileFromName)
{
    int fdFrom = open (fileFromName, 0);
    if (fdFrom == -1)
    {
        printf ("line  = %d\n", __LINE__);
        perror ("Error with open\n");
        return -1;
    }

    struct package* packagePtr = 0;
    packagePtr = (struct packagePtr*) shMemPtr;
    int readRetValue = 0;

    int initRet = initializeSemafores(semId);
    printf ("semaphores initialized ret = %d\n", initRet);

    struct sembuf sops [4];
    int semopRet = 0;

    PRINT ("1-in\n")
    dumpSemafores(semId);
    sem_do3 (NO_STR_FOR_STR, 0, 0,                  // checking if prev streamer and follower are out
             NO_FOL_FOR_STR, 0, 0,
             NO_STR_FOR_STR, 1, SEM_UNDO)           // close yourself. Only one streamer can exist

    PRINT ("2-in\n")
    dumpSemafores(semId);
    sem_do3 (NO_FOL_FOR_FOL, -1, 0,                 // checking if a NEW follower triggered first sem_do3
             NO_FOL_FOR_FOL,  1, 0,
             NO_STR_FOR_FOL,  1, SEM_UNDO)

    PRINT ("3-in\n")
    dumpSemafores(semId);

    int counter = 0;

    do
    {
        //produce_item();

        PRINT ("inside-1\n")
        // dumpSemafores (semId);
        // sleep (3);
        // dumpSemafores (semId);
        // sem_do  (EMPTY        , -1, 0)

        sem_do3 (NO_FOL_FOR_STR, -1, IPC_NOWAIT,    // the order is dramatically important =(
                 NO_FOL_FOR_STR,  1, IPC_NOWAIT,
                 EMPTY         , -1, 0)

        PRINT ("inside-2\n")
        dumpSemafores (semId);
        // sem_do  (MUT_EX        , -1, 0)

        sem_do3 (NO_FOL_FOR_STR,  1, IPC_NOWAIT,
                 NO_FOL_FOR_STR, -1, IPC_NOWAIT,
                 MUT_EX        , -1, SEM_UNDO)

        //put_item();
        //-----------------------------start of a critical section----------------------------
        //--------------streamer and follower, compete for an access to shmem---------------

        readRetValue = read (fdFrom, packagePtr->buffer, SIZE_OF_SHARED_BUFFER);
        if (readRetValue == -1)
        {
            perror ("problems with read\n");
            return -1;
        }
        packagePtr->size = readRetValue;
        PRINT ("thrown, size = %d\n", packagePtr->size);

        //sleep(1);
        //-----------------------------end of a critical section---------------------------------

        sem_do (MUT_EX,  1, SEM_UNDO)
        sem_do (FULL  ,  1, 0)

        counter++;
    }while (readRetValue != 0);


    PRINT ("nSemChanges = %d\n", counter)

    /*
    PRINT("1-out\n")
    sem_do(NO_STR_FOR_FOL, -1, 0)
    PRINT("2-out\n")
    sem_do(NO_STR_FOR_STR, -1, 0)
    PRINT("3-out\n")
    */

    PRINT ("1-out\n")
    dumpSemafores(semId);
    // sem_do2 (NO_STR_FOR_FOL, -1, 0,
    //          NO_STR_for_STR, -1, SEM_UNDO)
    // PRINT ("2-out\n")
    // dumpSemafores(semId);

    return 0;
}

//-----------------------------------------------------------------------------------------
//                      mode = 1
int follower (void* shMemPtr, int semId)
{
    struct package* packagePtr = (struct packagePtr*) shMemPtr;
    int writeRetValue = 0;

    int initRet = initializeSemafores(semId);
    printf ("semaphores initialized ret = %d\n", initRet);

    struct sembuf sops [4];
    int semopRet = 0;

    /*
    PRINT ("1-in\n")
    sem_do2 (NO_FOL_FOR_FOL, 0, 0, NO_FOL_FOR_FOL, 1, 0)
    PRINT ("2-in\n")
    sem_do2 (NO_STR_FOR_FOL, 0, 0, NO_STR_FOR_FOL, 1, 0)
    PRINT ("3-in\n")
	*/

    PRINT ("1-in\n")
	dumpSemafores(semId);
    sem_do3 (NO_FOL_FOR_FOL, 0, 0,
             NO_STR_FOR_FOL, 0, 0,
             NO_FOL_FOR_FOL, 1, SEM_UNDO)

    PRINT ("2-in\n")
    dumpSemafores(semId);
    sem_do3 (NO_STR_FOR_STR, -1, 0,
             NO_STR_FOR_STR,  1, 0,
             NO_FOL_FOR_STR,  1, SEM_UNDO)

     PRINT ("3-in\n")
     dumpSemafores(semId);

    int counter = 0;

    do
    {
        // sem_do  (FULL          , -1, 0)s
        sem_do3(NO_STR_FOR_FOL, -1, IPC_NOWAIT,             // the order is dramatically important
                NO_STR_FOR_FOL,  1, IPC_NOWAIT,
                FULL          , -1, 0)

        // sem_do  (MUT_EX        , -1, 0)
        sem_do3(NO_STR_FOR_FOL, -1, IPC_NOWAIT,
                NO_STR_FOR_FOL,  1, IPC_NOWAIT,
                MUT_EX        , -1, SEM_UNDO)

        //get_item ()
        //-----------------------Start of a critical section-----------------------------
        //----------------streamer and follower, compete for an access to shmem---------------


        writeRetValue = write (STDOUT_FILENO, packagePtr->buffer, packagePtr->size);
        if (writeRetValue == -1)
        {
            perror ("Error with write\n");
            return -1;
        }


        usleep (100000);
        // dumpSemafores(semId);
        // exit (0);
        //-----------------------End of a critical section-------------------------------
        sem_do (MUT_EX,  1, SEM_UNDO)
        sem_do (EMPTY ,  1, 0)
        //consume_item()

        counter++;


    }while (packagePtr->size != 0);

    PRINT ("nSemChanges = %d\n", counter)

    /*
    PRINT("1-out\n")
    sem_do(NO_FOL_FOR_FOL, -1, 0)
    PRINT("2-out\n")
    sem_do(NO_FOL_FOR_STR, -1, 0)
    PRINT("3-out\n")
    */

    PRINT("1-out\n")
    dumpSemafores(semId);
    // sem_do2 (NO_FOL_FOR_FOL, -1, SEM_UNDO,
    //          NO_FOL_FOR_STR, -1, 0)
    // PRINT("2-out\n")
    // dumpSemafores(semId);

    return 0;
}

//------------------------------------------------------------------------------------------------------

int initializeSemafores (int semId)
{
    struct sembuf sops [NSEMAFORES + 1];

    sops[0].sem_num = INITIALIZATOR;
    sops[0].sem_op  = 0;
    sops[0].sem_flg = IPC_NOWAIT;

    sops[1].sem_num = MUT_EX;
    sops[1].sem_op  = 1;
    sops[1].sem_flg = 0;

    sops[2].sem_num = EMPTY;
    sops[2].sem_op  = 1;
    sops[2].sem_flg = 0;

    int semopRetVal = semop (semId, sops, 3);    //If you add a new initialization of a semafore don't forget to change nsops

    sops[0].sem_num = INITIALIZATOR;
    sops[0].sem_op  = 1;
    sops[0].sem_flg = 0;

    semop (semId, sops, 1);

    return semopRetVal;
}

//--------------------------------------------------------------------------------------------------------

int dumpSemafores (int semId)
{
    printf ("------dump------\n");
    printf ("INITIALIZATOR  = %d\n", semctl(semId, INITIALIZATOR  , GETVAL));
    printf ("NO_STR_FOR_STR = %d\n", semctl(semId, NO_STR_FOR_STR , GETVAL));
    printf ("NO_FOL_FOR_STR = %d\n", semctl(semId, NO_FOL_FOR_STR , GETVAL));
    printf ("NO_STR_FOR_FOL = %d\n", semctl(semId, NO_STR_FOR_FOL , GETVAL));
    printf ("NO_FOL_FOR_FOL = %d\n", semctl(semId, NO_FOL_FOR_FOL , GETVAL));
    printf ("MUT_EX         = %d\n", semctl(semId, MUT_EX         , GETVAL));
    printf ("FULL           = %d\n", semctl(semId, FULL           , GETVAL));
    printf ("EMPTY          = %d\n", semctl(semId, EMPTY          , GETVAL));
    printf ("----------------\n");

    fflush (stdout);

    return 0;
}

//------------------------------------------------------------------------------------------------------

int modeDetection (char* mode)
{
    int val = 0;
    errno = 0;
    char* endptr = 0;

    val = strtol (mode, &endptr, 10);

    if (errno == ERANGE)
    {
        printf ("overflow of int\n");
        return -1;
    }
    else if (errno == EINVAL)
    {
        printf ("no digits seen\n");
        return -1;
    }
    else if (val < 0)
    {
        printf ("less then zero");
        return -1;
    }
    else if (*endptr != '\0')
    {
        printf ("I found not a digit\n");
        return -1;
    }

    if ((val == 0) || (val == 1))
        return val;
    else
        return -1;

}

//-------------------------------------------------------------------







/*

   int main (int argc, char** argv)
   {
   if ((argc > 3) || (argc == 1))
   {
   printf ("wrong number of arguments\n");
   return 0;
   }
   else if (argc == 0)
   {
   printf ("something wrong with argv pointer\n");
   return 0;
   }

   int mode = modeDetection (argv[1]);
//printf ("mode = %d\n", mode);
if (mode == 0) // streamer
{
if (argc == 2)
{
printf ("no file\n");
return 0;
}

char followerName [NAME_PID_LEN] = {};

int rdQueFd = open ("readersque", O_RDWR);
if (rdQueFd == -1)
{
printf ("error with opening readersque\n");
return -1;
}

int readRet = read (rdQueFd, followerName, NAME_PID_LEN);


//if a process dies here other end won't close

//int fdTo = open (followerName, O_WRONLY);
int fdTo = open (followerName, O_WRONLY | O_NONBLOCK);
if (errno == ENXIO)
{
printf ("errno == ENXIO\n");
return -1;
}

char buffer [BUFFER_LENGHT] = {};

int fdFrom = open (argv [2], 0);

int readRetValue = 1;
int writeRetValue = 0;
while (readRetValue != 0)
{
readRetValue  = read  (fdFrom, buffer, BUFFER_LENGHT);
if (readRetValue != 0)
{
writeRetValue = write (fdTo, buffer, readRetValue);
}
}

close (fdFrom);
close (fdTo);
close (rdQueFd);
return 0;

}
else if (mode == 1) // follower
{
if (argc == 3)
{
    printf ("I dont need any more arguments in this mode\n");
    return 0;
}

char namePid [NAME_PID_LEN] = {};
sprintf (namePid, "%d", getpid());
mkfifo (namePid, 0644);

//
int rdQueFd = open ("readersque", O_WRONLY);
write (rdQueFd, namePid, NAME_PID_LEN);
//


int fdFrom = open (namePid, O_RDONLY);
//fcntl (fdFrom, F_SETFL, O_RDONLY | O_NONBLOCK);


char buffer [BUFFER_LENGHT] = {};

int readRetValue = BUFFER_LENGHT;

while (readRetValue == BUFFER_LENGHT)
{
    readRetValue = read (fdFrom, buffer, BUFFER_LENGHT);
    if (readRetValue > 0)
        write (STDOUT_FILENO, buffer, readRetValue);
}


close  (fdFrom);

close (rdQueFd);
unlink (namePid);
return 0;
}
else
{
    printf ("some mistake occured\n");
    return 0;
}

return 0;
}



*/
