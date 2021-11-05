
#include "local.h"

void signal_alarm_catcher(int the_sig);
void access_denied();

static struct sembuf acquire = {0, -1, SEM_UNDO},
                     release = {0, 1, SEM_UNDO};

int access_denied_count;
int main(int argc, char *argv[])
{

    int passport_stamp, passenger_pid, random_alarm;
    char SEED = *argv[0], recieved_msg[10], valid_passport[2];
    long mid, hall_mid;
    key_t key, hall_key;
    MESSAGE msg;

    access_denied_count = atoi(argv[1]);

    srand(getpid());

    // Alarm signal catcher
    // Alarm used to calculate time in of terminated case for project
    if (sigset(SIGALRM, signal_alarm_catcher) == SIGALRM)
    {
        perror("Sigset can not set SIGINT");
        exit(SIGSTOP);
    }

    // Random alarm value (40-55 Seconds)
    random_alarm = (rand() % 15) + 150;
    alarm(random_alarm);
    sleep(1);

    printf("the officer id is: %d, with seed = %c \n", getpid(), SEED);
    fflush(stdout);

    // Create V key used to create message queue
    if ((key = ftok(".", SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    /*
    * Create message queue
    */
    if ((mid = msgget(key, 0)) == -1)
    {
        mid = msgget(key, IPC_CREAT | 0660);
    }

    if ((hall_key = ftok(".", HALL_SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((hall_mid = msgget(hall_key, 0)) == -1)
    {
        hall_mid = msgget(hall_key, IPC_CREAT | 0660);
    }

    while (1)
    {
        if (msgrcv(mid, &msg, MSGSZ, 1, 0) == -1)
        {
            perror("Client: msgsend");
            return 4;
        }
        printf("From officer with seed = %c : %s \n", SEED, msg.mtext);
        fflush(stdout);

        // Random process length between 3 and 5
        passport_stamp = (rand() % 3) + 3;
        strcpy(recieved_msg, msg.mtext);

        char *token = strtok(recieved_msg, "-");
        passenger_pid = atoi(token);
        token = strtok(NULL, "-");
        strcpy(valid_passport, token);

        // Inform passenger he reached the officer
        // Check if passenger is still alive
        if (kill(passenger_pid, SIGUSR1) != -1)
        {
            if (strcmp(valid_passport, "T") == 0)
            {
                sleep(passport_stamp);

                MESSAGE msg_to_hall;
                char str_passenger_pid[12];

                msg_to_hall.mtype = HALL_MESSAGE_TYPE;
                sprintf(str_passenger_pid, "%d", passenger_pid);
                strcpy(msg_to_hall.mtext, str_passenger_pid);

                if (msgsnd(hall_mid, &msg_to_hall, HALL_MESSAGE_SIZE, 0) == -1)
                {
                    perror("Client: msgsend");
                    return 4;
                }

                printf("send passenger (%d) to hall\n", passenger_pid);
                fflush(stdout);
            }
            else
            {
                // Invalid Passport -> takes 0.5 * Passport Stamp
                sleep((int)(passport_stamp / 2));
                int r = kill(passenger_pid, SIGTERM);
            }
        }
    }

    return 0;
}

// SIGALRM Catcher
void signal_alarm_catcher(int the_sig)
{
    exit(1);
}

void access_denied()
{
    long semid;
    key_t key;

    key = ftok(".", SEM_ACCESS_DENIED_SEED);
    if ((semid = semget(key, 1, IPC_EXCL | 0660)) != -1)
    {
        printf(" acceess granted connected\n");
    }

    if (semop(semid, &acquire, 1) == -1)
    {
        perror("semop -- producer -- waiting for consumer to read number");
        exit(3);
    }

    // TODO: increment the access granted
    char *shmptr;
    int tmp;
    long shmid;

    if ((key = ftok(".", ACCESS_DENIED_SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((shmid = shmget(key, 10, 0666)) != -1)
    {
        if ((shmptr = (char *)shmat(shmid, 0, 0)) == (char *)-1)
        {
            perror("shmptr -- parent -- attach");
            exit(1);
        }
        tmp = atoi(shmptr);
        tmp++;
        sprintf(shmptr, "%d", tmp);
        printf(" access denied is: ------------------------ (%s) ---------------------\n", shmptr);
        shmdt(shmid);
    }
    else
    {
        perror("shm attach");
    }

    if (semop(semid, &release, 1) == -1)
    {
        perror("semop -- producer -- indicating new number has been made");
        exit(5);
    }

    if (tmp == access_denied_count){
        kill(getppid(), SIGUSR1);
    }
}