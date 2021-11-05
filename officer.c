
#include "local.h"

void signal_alarm_catcher(int the_sig);

int main(int argc, char *argv[])
{
    key_t key, hall_key;
    char SEED = *argv[0];
    long mid, hall_mid;
    MESSAGE msg;
    int passport_stamp, passenger_pid, random_alarm;
    char recieved_msg[10], valid_passport[2];

    srand(getpid());

    if (sigset(SIGALRM, signal_alarm_catcher) == SIGALRM)
    {
        perror("Sigset can not set SIGINT");
        exit(SIGSTOP);
    }

    // Random alarm value (15-30 Seconds)
    random_alarm = (rand() % 15) + 15;
    alarm(random_alarm);

    sleep(1);

    printf("the officer id is: %d, with seed = %c \n", getpid(), SEED);
    fflush(stdout);
    if ((key = ftok(".", SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

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
                msg_to_hall.mtype = HALL_MESSAGE_TYPE;
                char str_passenger_pid[12];
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
// SWGALRM Catcher
void signal_alarm_catcher(int the_sig)
{
    exit(1);
}