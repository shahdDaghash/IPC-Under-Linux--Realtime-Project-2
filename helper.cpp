#include "local.h"
using namespace std;

void createSharedMemory();
void openSemaphores();

struct MEMORY *sharedMemory;
int shmid, r_semid, w_semid;
int numOfColumns;

static struct sembuf acquire = {0, -1, SEM_UNDO},
                     release = {0, 1, SEM_UNDO};

int main()
{
    srand(getpid());
    unsigned col1, col2;
    createSharedMemory();
    openSemaphores();

    int i = 10; // TODO: remove

    while (1 && i--)
    {
        col1 = rand() % numOfColumns;
        col2 = rand() % numOfColumns;
        if (col1 > col2)
        {
            swap(col1, col2);
        }
        else if (col1 == col2)
        {
            continue;
        }

        acquire.sem_num = col1;
        cout << "HELPER " << getpid() << " aq " << col1 << endl;
        if (semop(w_semid, &acquire, 1) == -1)
        {
            perror("HELPER: semop write sem1");
            exit(3);
        }
        if (semop(r_semid, &acquire, 1) == -1)
        {
            perror("HELPER: semop read sem1");
            exit(3);
        }

        cout << "HELPER " << getpid() << " aq " << col2 << endl;
        acquire.sem_num = col2;
        if (semop(w_semid, &acquire, 1) == -1)
        {
            perror("HELPER: semop write sem2");
            exit(3);
        }
        if (semop(r_semid, &acquire, 1) == -1)
        {
            perror("HELPER: semop read sem2");
            exit(3);
        }

        cout << "HELPER " << getpid() << " is writting" << endl;

        char temp[MAX_STRING_LENGTH];
        strcpy(temp, sharedMemory->data[col1]);
        strcpy(sharedMemory->data[col1], sharedMemory->data[col2]);
        strcpy(sharedMemory->data[col2], temp);
        cout << "HELPER " << getpid() << " finished writting" << endl;

        release.sem_num = col1;
        cout << "HELPER " << getpid() << " re " << col1 << endl;
        if (semop(r_semid, &release, 1) == -1)
        {
            perror("HELPER: semop read sem1");
            exit(3);
        }
        if (semop(w_semid, &release, 1) == -1)
        {
            perror("HELPER: semop write sem1");
            exit(3);
        }

        cout << "HELPER " << getpid() << " re " << col2 << endl;
        release.sem_num = col2;
        if (semop(r_semid, &release, 1) == -1)
        {
            perror("HELPER: semop read sem2");
            exit(3);
        }
        if (semop(w_semid, &release, 1) == -1)
        {
            perror("HELPER: semop write sem2");
            exit(3);
        }

        sleep(rand() % 3); // TODO:: REMOVE?
    }
    return 0;
}

void createSharedMemory()
{
    key_t key = ftok(".", MEM_SEED);
    if (key == -1)
    {
        perror("HELPER: key generation");
        exit(3);
    }
    if ((shmid = shmget(key, 0, 0)) == -1)
    {
        perror("HELPER: shmid");
        exit(3);
    }
    // Attach the shared memory segment
    if ((sharedMemory = (struct MEMORY *)shmat(shmid, NULL, 0)) == (struct MEMORY *)-1)
    {
        perror("HELPER: shmat");
    }
    numOfColumns = sharedMemory->rows;
}
void openSemaphores()
{
    int *semid[] = {&r_semid, &w_semid};
    int seeds[] = {SEM_R_SEED, SEM_W_SEED};

    key_t key;
    for (int i = 0; i < 2; i++)
    {
        if ((key = ftok(".", seeds[i])) == -1)
        {
            perror("HELPER: semaphore key generation");
            exit(1);
        }
        if ((*semid[i] = semget(key, numOfColumns, 0)) == -1)
        {
            perror("HELPER: semget obtaining semaphore");
            exit(2);
        }
    }
}