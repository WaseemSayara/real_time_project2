
#include "local.h"

MESSAGE create_message();
void signal_alarm_catcher(int);
void signal_usr1_catcher(int);
void signal_in_valid_passport_catcher(int);
void impatient();

static struct sembuf acquire = {0, -1, SEM_UNDO},
                     release = {0, 1, SEM_UNDO};

int impatient_count;

int main(int argc, char *argv[])
{
    key_t key;
    char SEED;
    int nationality, officers, arab_officers, random_alarm;
    long mid;
    MESSAGE msg;

    srand(getpid());

    /*
    * Signal Catchers
    */

    if (sigset(SIGALRM, signal_alarm_catcher) == SIGALRM)
    {
        perror("Sigset can not set SIGALRM");
        exit(SIGALRM);
    }

    if (sigset(SIGUSR1, signal_usr1_catcher) == SIGUSR1)
    {
        perror("Sigset can not set SIGUSR1");
        exit(SIGUSR1);
    }

    if (sigset(SIGTERM, signal_in_valid_passport_catcher) == SIGTERM)
    {
        perror("Sigset can not set SIGKILL");
        exit(SIGTERM);
    }

    // Random alarm value (45-60 Seconds)
    random_alarm = (rand() % 15) + 45;
    alarm(random_alarm);

    // Random Nationality with user-defined Percentage
    int tmp = (rand() % 100);
    if (tmp <= 100 * ARAB_PERCENTS)
        // Arab
        nationality = 0;
    else
        // Forigen
        nationality = 1;

    officers = atoi(argv[0]);
    arab_officers = officers * ARAB_PERCENTS;

    impatient_count = atoi(argv[1]);

    if (nationality == 0)
    {
        tmp = (rand() % arab_officers);
        SEED = 'A' + tmp;
    }
    else if (nationality == 1)
    {
        tmp = (rand() % (officers - arab_officers));
        SEED = 'A' + arab_officers + tmp;
    }

    if ((key = ftok(".", SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((mid = msgget(key, 0)) == -1)
    {
        mid = msgget(key, IPC_CREAT | 0660);
    }

    msg.mtype = 1;

    msg = create_message(nationality);

    int buf_length = strlen(msg.mtext) + 1;

    if (msgsnd(mid, &msg, buf_length, 0) == -1)
    {
        perror("Client335: msgsend");
        return 4;
    }

    while (1)
    {
        // to keep the passenger alive until he reaches the borders
        pause();
    }

    return 0;
}

// Function to create message with text "PID-(T/F)" and type 1
MESSAGE create_message(int nat)
{

    char valid_passport[2];
    char str_pid[7];
    char message[12];
    srand(getpid());
    MESSAGE tmp_msg;
    char nataonality[10];
    char passport[10];

    if (nat == 0)
    {
        strcpy(nataonality, "Arab");
    }
    else
    {
        strcpy(nataonality, "Foreign");
    }

    // Random Valid Passport value with Validity Percentage %90
    int tmp = (rand() % 100);
    if (tmp <= 90)
    {
        strcpy(valid_passport, "T");
        strcpy(passport, "Valid");
        green();
        printf("Passenger with pid = %d, is %s, and his passport is %s\n", getpid(), nataonality, passport);
        fflush(stdout);
        reset();
    }
    else
    {
        strcpy(valid_passport, "F");
        strcpy(passport, "InValid");
        green();
        printf("Passenger with pid = %d, is %s, and his passport is %s\n", getpid(), nataonality, passport);
        fflush(stdout);
        reset();
    }

    sprintf(str_pid, "%d", getpid());

    // Return message consist pid-Passport validity either T (True) or F (False)
    strcat(message, str_pid);
    strcat(message, "-");
    strcat(message, valid_passport);

    tmp_msg.mtype = 1;
    strcpy(tmp_msg.mtext, message);

    return tmp_msg;
}

// SIGALRM catcher
void signal_alarm_catcher(int the_sig)
{
    red();
    printf("passenger with pid = %d, got impatient and exited\n", getpid());
    fflush(stdout);
    reset();
    impatient();
    exit(1);
}

// SIGUSR1 catcher
void signal_usr1_catcher(int the_sig)
{
    // Cancel Current Alarms
    alarm(0);
}

// SIGTERM catcher
void signal_in_valid_passport_catcher(int the_sig)
{
    red();
    printf("passenger with pid = %d, exited due to inValid passport\n", getpid());
    fflush(stdout);
    reset();
    exit(2);
}

void impatient()
{
    long semid;
    key_t key;

    key = ftok(".", SEM_IMPATIENT_SEED);
    if ((semid = semget(key, 1, IPC_EXCL | 0660)) == -1)
    {
        perror(" cant access semaphore");
    }

    if (semop(semid, &acquire, 1) == -1)
    {
        perror("semop -- producer -- waiting for consumer to read number passenger ");
        return;
    }

    char *shmptr;
    int tmp;
    long shmid;

    if ((key = ftok(".", IMPATIENT_SEED)) == -1)
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
        printf(" Impatient count is: ------------------------ (%s) ---------------------\n", shmptr);
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

    if (tmp == impatient_count)
    {
        int parent = getppid();
        if (parent > 10)
            kill(getppid(), SIGINT);
    }
}
