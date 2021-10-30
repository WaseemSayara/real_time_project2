
#include "local.h"


int main(int argc, char *argv[])
{
    key_t key;
    char SEED = *argv[0];
    long mid;
    MESSAGE msg;

    printf("the officer id is: %d, with seed = %c \n", getpid(), SEED);
    if ((key = ftok(".", SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((mid = msgget(key, 0 )) == -1 ) {        
    mid = msgget(key,IPC_CREAT | 0660);
    }

    if (SEED == 'A') {
    if (msgrcv(mid, &msg,MSGSZ,1, 0) == -1 ) {
      perror("Client: msgsend");
      return 4;
    }
        printf("From officer : %s \n", msg.mtext);
    }


    return 0;
}