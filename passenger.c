
#include "local.h"

MESSAGE create_message();
void signal_alarm_catcher(int);
void signal_usr1_catcher(int);
void signal_in_valid_passport_catcher(int);

int main(int argc, char *argv[])
{
    key_t key;
    char SEED;
    int nationality;
    int officers, arab_officers;
    long mid;
    MESSAGE msg;

    srand(getpid());

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

    alarm(30);

    int tmp = (rand() % 100);
    if (tmp <= 70)
        nationality = 0; // arab
    else
        nationality = 1; // forigen

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

    // sleep(1);

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

MESSAGE create_message()
{

    char valid_passport[2];
    char str_pid[7];
    char message[12];
    srand(getpid());
    MESSAGE tmp_msg;

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

void signal_alarm_catcher(int the_sig)
{
    printf("passenger with pid = %d, exited from alarm\n", getpid());
    fflush(stdout);
    exit(1);
}

void signal_usr1_catcher(int the_sig)
{
    alarm(0); // cancel current alarms
}

void signal_in_valid_passport_catcher(int the_sig)
{
    printf("passenger with pid = %d, exited due to inValid passport\n", getpid());
    fflush(stdout);
    exit(2);
}
