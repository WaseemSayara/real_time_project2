
#include "local.h"

int main(int argc, char *argv[])
{
    key_t key;
    char SEED = 'A';
    long mid;
    MESSAGE msg;

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

    strcpy(msg.mtext, "19442-t");
    msg.mtype = 1;
    printf("From passenger : %s \n", msg.mtext);

    sleep(1);

    int buf_length = strlen(msg.mtext) + 1;

    if (msgsnd(mid, &msg, buf_length, 0) == -1)
    {
        perror("Client: msgsend");
        return 4;
    }

    return 0;
}