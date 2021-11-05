
#include "local.h"

MESSAGE create_message();
void signal_alarm_catcher(int);
void signal_usr1_catcher(int);
void signal_in_valid_passport_catcher(int);

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

    // Random alarm value (15-30 Seconds)
    random_alarm = (rand() % 15) + 15;
    alarm(random_alarm);

    // Random Nationality with user-defined Percentage
    int tmp = (rand() % 1);
    if (tmp <= ARAB_PERCENTS)
        // Arab
        nationality = 0;
    else
        // Forigen
        nationality = 1;

    officers = atoi(argv[0]);
    arab_officers = officers * ARAB_PERCENTS;

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

    msg = create_message();

    printf("From passenger : %s , with seed = %c \n", msg.mtext, SEED);
     fflush(stdout);

    int buf_length = strlen(msg.mtext) + 1;

    if (msgsnd(mid, &msg, buf_length, 0) == -1)
    {
        perror("Client: msgsend");
        return 4;
    }

    while (1)
    {
        pause();
        printf("Passenger with id = %d , got signal\n", getpid());
        fflush(stdout);
    }

    return 0;
}

// Function to create message with text "PID-(T/F)" and type 1
MESSAGE create_message()
{

    char valid_passport[2];
    char str_pid[7];
    char message[12];
    srand(getpid());
    MESSAGE tmp_msg;

    // Random Valid Passport value with Validity Percentage %90
    int tmp = (rand() % 100);
    if (tmp <= 90)
        strcpy(valid_passport, "T");
    else
        strcpy(valid_passport, "F");

    sprintf(str_pid, "%d", getpid());

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
    printf("passenger with pid = %d, exited from alarm\n", getpid());
    fflush(stdout);
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
    printf("passenger with pid = %d, exited due to inValid passport\n", getpid());
    fflush(stdout);
    exit(2);
}
