
#include "local.h"

MESSAGE create_message();

int main(int argc, char *argv[])
{
    key_t key;
    char SEED;
    int nationality;
    int officers, arab_officers;
    long mid;
    MESSAGE msg;

    srand(getpid());

    int tmp = (rand() % 100);
    if (tmp <= 70)
        nationality = 0; // arab
    else
        nationality = 1; // forigen

    strcpy (nationality , argv[0]);
    officers = atoi(argv[0]);
    arab_officers = officers * ARAB_PERCENTS;

    if (nationality == 0) {
        tmp = (rand() % arab_officers);
        SEED = 'A' + tmp ;
    }
    else if (nationality == 1) {
        tmp = (rand() % (officers - arab_officers) );
        SEED = 'A' + arab_officers + tmp ;
    }

    printf("the passenger id is: %d, with seed = %c \n", getpid(), SEED);
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

    printf("From passenger : %s \n", msg.mtext);

    // sleep(1);

    // int buf_length = strlen(msg.mtext) + 1;

    // if (msgsnd(mid, &msg, buf_length, 0) == -1)
    // {
    //     perror("Client: msgsend");
    //     return 4;
    // }

    return 0;
}

MESSAGE create_message()
{

    char valid_passport[2];
    char str_pid[5];
    char message[10];
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