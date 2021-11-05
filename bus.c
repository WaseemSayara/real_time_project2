
#include "local.h"

void enQueue(struct Queue *, int);
int deQueue(struct Queue *);
struct QNode *newNode(int id);
struct Queue *createQueue();
void signal_alarm_catcher(int);
void signal_ready_catcher(int);
void signal_end_catcher(int);
void empty_bus(int);

long mid;
int ready_flag = 0, num_of_busses, capacity_of_bus, bus_num;
struct Queue *passengers;

int main(int argc, char *argv[])
{

    if (sigset(SIGALRM, signal_alarm_catcher) == SIGALRM)
    {
        perror("Sigset can not set SIGINT");
        exit(SIGSTOP);
    }
    if (sigset(SIGUSR1, signal_ready_catcher) == SIGUSR1)
    {
        perror("Sigset can not set SIGUSR1");
        exit(SIGSTOP);
    }
    if (sigset(SIGINT, signal_end_catcher) == SIGINT)
    {
        perror("Sigset can not set SIGUSR1");
        exit(SIGINT);
    }

    MESSAGE msg;
    int passenger_id, bus_id, sleep_time;
    char tmp[20];
    bus_id = getpid();

    srand(bus_id);

    bus_num = atoi(argv[0]);

    printf("the bus id is: %d, and number is %d \n", getpid(), bus_num);

    FILE *variables = fopen("variables.txt", "r");
    fscanf(variables, "%s %d\n", &tmp, &num_of_busses);
    fscanf(variables, "%s %d\n", &tmp, &capacity_of_bus);

    if ((mid = msgget(bus_id, 0)) == -1)
    {
        mid = msgget(bus_id, IPC_CREAT | 0660);
    }
    alarm(150);

    passengers = createQueue();

    while (1)
    {

        if (ready_flag == 0)
        {

            if (msgrcv(mid, &msg, BUS_MESSAGE_SIZE, BUS_MESSAGE_TYPE, 0) == -1)
            {
                perror("Client: msgrcv ");
            }
            else
            {
                passenger_id = atoi(msg.mtext);
                printf("Bus num : %d , recived passenger %d \n", bus_num, passenger_id);
                enQueue(passengers, passenger_id);
            }
        }
        else if (ready_flag == 1)
        {
            sleep_time = rand() % 4 + 15;
            sleep(sleep_time);
            while (1)
            {
                passenger_id = deQueue(passengers);
                // queue is empty
                if (passenger_id == -1)
                {
                    printf("BUSS %d is entered break\n", bus_num);
                    fflush(stdout);
                    break;
                }
                kill(passenger_id, SIGKILL);
            }
            sleep(sleep_time);
            empty_bus(bus_num);
            ready_flag == 0;
            printf("BUSS %d is ready to load\n", bus_num);
            fflush(stdout);
        }
    }

    return 0;
}

void clean()
{
    int passenger_id;
    while (1)
    {
        passenger_id = deQueue(passengers);
        // queue is empty
        if (passenger_id == -1)
        {
            msgctl(mid, IPC_RMID, (struct msqid_ds *)0);
            printf("BUS %d IS CLEAN NOW\n", bus_num);
            return 0;
        }
        kill(passenger_id, SIGKILL);
    }
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

void empty_bus(int bus_num)
{

    key_t key, sem_array_key;
    long sem_array_id;
    int sem_value;

    sem_array_key = ftok(".", SEM_ARRAY_SEED);
    printf(" ------  Bus Semaphore: %d\n", sem_array_key);
    fflush(stdout);

    /*
* Create the semaphore
*/

    struct semid_ds sem_buf;
    ushort sem_array[num_of_busses];
    union semun arg;

    if ((sem_array_id = semget(sem_array_key, num_of_busses, 0660)) == -1)
    {
        return;
    }

    printf("Semaphore identifier %d\n", sem_array_id);
    fflush(stdout);

    if (semctl(sem_array_id, bus_num, SETVAL, capacity_of_bus) == -1)
    {
        perror("SETVAL error");
    }
}

void signal_alarm_catcher(int the_sig)
{
    msgctl(mid, IPC_RMID, (struct msqid_ds *)0);
    exit(1);
}

void signal_ready_catcher(int the_sig)
{
    ready_flag = 1;
    printf("BUSS %d is full and going to unload\n", bus_num);
}

void signal_end_catcher(int the_sig)
{
    ready_flag = 2;
    clean();
}
