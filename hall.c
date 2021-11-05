
#include "local.h"

void enQueue(struct Queue *, int);
int deQueue(struct Queue *);
struct QNode *newNode(int id);
struct Queue *createQueue();
void signal_alarm_catcher(int);
void increment_shared_memory(long);
int hall_count = 0;

int main(int argc, char *argv[])
{
    
    long mid, shmid_1, shmid_2, shmid_3, bus_sem_array_id;
    int max_limit, min_limit, num_of_busses;
    char str_passenger_id[10], tmp[20];
    struct msqid_ds buf;
    MESSAGE recieved_msg, to_bus_msg;
    key_t key;
    FILE *variables;
    
    // Received threshold limits for hall
    max_limit = atoi(argv[0]);
    min_limit = atoi(argv[1]);

    printf("the hall id is: %d\n", getpid());

    // Sigalarm catcher
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
        perror("shmid1 -- parent -- creation 666666666");
        exit(2);
    }

    alarm(150);

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

    pid_t bus_pid_array[num_of_busses], bus_msg_array[num_of_busses];
    char str_bus_ids[num_of_busses * 10];

    strcpy(str_bus_ids, argv[2]);

    printf(" 999 %s 999\n", str_bus_ids);

    char *pch = strtok(str_bus_ids, "-");
    bus_pid_array[0] = atoi(pch);

    for (int i = 1; i < num_of_busses; i++)
    {

        pch = strtok(NULL, "-");
        bus_pid_array[i] = atoi(pch);
    }

    for (int i = 0; i < num_of_busses; i++)
    {
        printf("INTEGER_PID : %d\n", bus_pid_array[i]);
    }

    for (int i = 0; i < num_of_busses; i++)
    {
        if ((bus_msg_array[i] = msgget(bus_pid_array[i], 0)) == -1)
        {
            perror("connect to bus");
        }
    }

    // 000000000000000000000000000000000000000000000000000000000000000000000000

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
        perror("shmid1 -- parent -- creation 55555555");
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
            yellow();
            printf("the current count in the hall is --( %d )--\n", hall_count);
            fflush(stdout);
            reset();
            increment_shared_memory(shmid_1);

            // check bus semaphores
            if ((current_value = semctl(bus_sem_array_id, current_bus, GETVAL, 0)) == -1)
            {
                perror("semctl: GETVAL");
                exit(4);
            }
            
            printf(" current bus: %d,  with sem_value = %d \n\n", current_bus, current_value);
            while (current_value > 0)
            {
                
                passenger_id = deQueue(passengers);
                sprintf(str_passenger_id, "%d", passenger_id);

                to_bus_msg.mtype = BUS_MESSAGE_TYPE;
                strcpy(to_bus_msg.mtext, str_passenger_id);

                if (msgsnd(bus_msg_array[current_bus], &to_bus_msg, BUS_MESSAGE_SIZE, 0) == -1)
                {
                    perror("Client: msgsend");
                    return 4;
                }
                printf(" ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ load in bus : %d\n", current_bus);

                hall_count--;
                current_value--;

                if (hall_count == 0 || current_value == 0)
                {
                    if (semctl(bus_sem_array_id, current_bus, SETVAL, current_value) == -1)
                    {
                        perror("SETVAL error");
                    }
                    int current_value2;
                    if ((current_value2 = semctl(bus_sem_array_id, current_bus, GETVAL, 0)) == -1)
                    {
                        perror("semctl: GETVAL");
                        exit(4);
                    }
                    printf("{{{{ sem after set is  %d  }}}} \n\n", current_value2);

                    if (current_value2 == 0){
                        printf(" ========================== send signal to bus : %d \n", current_bus);
                        kill(bus_pid_array[current_bus], SIGUSR1);
                    }

                    break;
                }
            }

            // if there is no space left in the bus -> go to next bus
            if (current_value == 0)
            {
                current_bus = (current_bus + 1) % num_of_busses;
                printf(" GGGGGGGoing to NEEEEEXT BUS\n");
            }
        }
    }

    return 0;
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

void increment_shared_memory(long shmid)
{
    char *shmptr;
    int tmp;

    if ((shmptr = (char *)shmat(shmid, 0, 0)) == (char *)-1)
    {
        perror("shmptr -- parent -- attach");
        exit(1);
    }
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