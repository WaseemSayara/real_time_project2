
#include "local.h"

// A linked list (LL) node to store a queue entry
struct QNode
{
    int passenger_pid;
    struct QNode *next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue
{
    struct QNode *front, *rear;
};

// A utility function to create a new linked list node.
struct QNode *newNode(int id)
{
    struct QNode *temp = (struct QNode *)malloc(sizeof(struct QNode));
    temp->passenger_pid = id;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue *createQueue()
{
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enQueue(struct Queue *q, int id)
{
    // Create a new LL node
    struct QNode *temp = newNode(id);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a key from given queue q
int deQueue(struct Queue *q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return -1;

    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;
    int temp_id = temp->passenger_pid;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
    return temp_id;
}

void signal_alarm_catcher(int);
int hall_count = 0;

int main(int argc, char *argv[])
{
    key_t key;
    long mid;
    struct msqid_ds buf;
    MESSAGE recieved_msg;
    int max_limit, min_limit;
    int shmid_1, shmid_2, shmid_3;
    char *shmptr1, *shmptr2, *shmptr3;

    max_limit = atoi(argv[0]);
    min_limit = atoi(argv[1]);

    printf("the hall id is: %d\n", getpid());

    if (sigset(SIGALRM, signal_alarm_catcher) == SIGALRM)
    {
        perror("Sigset can not set SIGALRM");
        exit(SIGALRM);
    }

    if ((key = ftok(".", ACCESS_GRANTED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((shmid_1 = shmget(key, 10, IPC_CREAT | 0666)) != -1)
    {

        if ((shmptr1 = (char *)shmat(shmid_1, 0, 0)) == (char *)-1)
        {
            perror("shmptr -- parent -- attach");
            exit(1);
        }
        printf("------------------------ (%s) ---------------------\n", shmptr1);
        
    }
    else
    {
        perror("shmid1 -- parent -- creation");
        exit(2);
    }

    alarm(60);

    struct Queue *passengers = createQueue();

    if ((key = ftok(".", HALL_SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((mid = msgget(key, 0)) == -1)
    {
        mid = msgget(key, IPC_CREAT | 0660);
    }

    msgctl(mid, IPC_STAT, &buf);

    buf.msg_qbytes = 12;
    msgctl(mid, IPC_SET, &buf);

    while (1)
    {
        if (hall_count < max_limit)
        {
            if (msgrcv(mid, &recieved_msg, HALL_MESSAGE_SIZE, HALL_MESSAGE_TYPE, 0) == -1)
            {
                perror("Client3 : msgsend");
                return 4;
            }
            int passenger_id = atoi(recieved_msg.mtext);

            printf("passenger (%d) is recived in to hall\n", passenger_id);
            fflush(stdout);

            enQueue(passengers, passenger_id);
            hall_count++;
            printf("the current count in the hall is --( %d )--\n", hall_count);
            fflush(stdout);
        }
    }

    return 0;
}

void signal_alarm_catcher(int the_sig)
{
    printf("Hall with pid = %d, exited from alarm\n", getpid());
    fflush(stdout);
    exit(1);
}