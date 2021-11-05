
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
void increment_shared_memory(long);
int hall_count = 0;

int main(int argc, char *argv[])
{
    key_t key;
    long mid;
    struct msqid_ds buf;
    MESSAGE recieved_msg, to_bus_msg;
    int max_limit, min_limit, num_of_busses;
    long shmid_1, shmid_2, shmid_3, bus_sem_array_id;
    FILE *variables;
    char str_passenger_id[10], tmp[20];

    max_limit = atoi(argv[0]);
    min_limit = atoi(argv[1]);

    printf("the hall id is: %d\n", getpid());

    if (sigset(SIGALRM, signal_alarm_catcher) == SIGALRM)
    {
        perror("Sigset can not set SIGALRM");
        exit(SIGALRM);
    }

    if ((key = ftok(".", ACCESS_GRANTED_SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((shmid_1 = shmget(key, 10, IPC_CREAT | 0666)) != -1)
    {
        printf("------------------------ ( shem connected ) ---------------------\n");
    }
    else
    {
        perror("shmid1 -- parent -- creation");
        exit(2);
    }

    alarm(80);

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

    variables = fopen("variables.txt", "r");
    fscanf(variables, "%s %d\n", &tmp, &num_of_busses);
    fclose(variables);

    if ((key = ftok(".", SEM_ARRAY_SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((bus_sem_array_id = semget(key, num_of_busses, 0)) != -1)
    {
        printf("------------------------ ( shem connected ) ---------------------\n");
    }
    else
    {
        perror("shmid1 -- parent -- creation");
        exit(2);
    }

    int current_bus = 0;
    int current_value;
    int passenger_id;

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
            increment_shared_memory(shmid_1);

            // check bus semaphores
            if ((current_value = semctl(bus_sem_array_id, current_bus, GETVAL, 0)) == -1)
            {
                perror("semctl: GETVAL");
                exit(4);
            }
            while (current_value >= 0)
            {
                if (hall_count == 0)
                    break;

                passenger_id = deQueue(passengers);
                sprintf(str_passenger_id, "%d", passenger_id);

                to_bus_msg.mtype = BUS_MESSAGE_TYPE;
                strcpy(to_bus_msg.mtext, str_passenger_id);

                // send id to bus queue

                hall_count--;
                current_value--;
            }

            if (semctl(bus_sem_array_id, current_bus, SETVAL, current_value) == -1)
            {
                perror("SETVAL error");
            }

            // if there is no space left in the bus -> go to next bus
            if (current_value == 0)
            {
                current_bus = (current_bus + 1) % num_of_busses;
            }
        }
    }

    return 0;
}

void increment_shared_memory(long shmid)
{
    char *shmptr;
    int tmp;

    if ((shmptr = (char *)shmat(shmid, 0, 0)) == (char *)-1)
    {
        perror("shmptr -- parent -- attach");
        exit(1);
    }
    printf(" mem was : ------------------------ (%s) ---------------------\n", shmptr);
    tmp = atoi(shmptr);
    tmp++;
    sprintf(shmptr, "%d", tmp);
    printf(" mem is now : ------------------------ (%s) ---------------------\n", shmptr);
    shmdt(shmid);
}

// SIGALRM Catcher
void signal_alarm_catcher(int the_sig)
{
    printf("Hall with pid = %d, exited from alarm\n", getpid());
    fflush(stdout);
    exit(1);
}