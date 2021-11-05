
#include "local.h"

void enQueue(struct Queue *, int);
int deQueue(struct Queue *);
struct QNode *newNode(int id);
struct Queue *createQueue();
void signal_end_catcher(int);
void signal_alarm_catcher(int);
void access_granted();

int hall_count = 0, end_flag = 0, start_min = 0;
int access_granted_count;
long mid;
static struct sembuf acquire = {0, -1, SEM_UNDO},
                     release = {0, 1, SEM_UNDO};

struct Queue *passengers;
int main(int argc, char *argv[])
{

    long shmid_1, shmid_2, shmid_3, bus_sem_array_id;
    int max_limit, min_limit, num_of_busses;
    char str_passenger_id[10], tmp[20];
    struct msqid_ds buf;
    MESSAGE recieved_msg, to_bus_msg;
    key_t key;
    FILE *variables;
    char str_bus_ids[num_of_busses * 10];

    // Received threshold limits for hall
    max_limit = atoi(argv[0]);
    min_limit = atoi(argv[1]);
    access_granted_count = atoi(argv[3]);

    // Sigalarm catcher
    if (sigset(SIGALRM, signal_alarm_catcher) == SIGALRM)
    {
        perror("Sigset can not set SIGALRM");
        exit(SIGALRM);
    }
    if (sigset(SIGINT, signal_end_catcher) == SIGINT)
    {
        perror("Sigset can not set SIGALRM");
        exit(SIGINT);
    }

    alarm(250);

    passengers = createQueue();

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

    strcpy(str_bus_ids, argv[2]);
    char *pch = strtok(str_bus_ids, "-");
    bus_pid_array[0] = atoi(pch);

    for (int i = 1; i < num_of_busses; i++)
    {

        pch = strtok(NULL, "-");
        bus_pid_array[i] = atoi(pch);
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

    if ((bus_sem_array_id = semget(key, num_of_busses, 0)) == -1)
    {
        perror("shmid1 -- parent -- creation 55555555");
        exit(2);
    }

    int current_bus = 0;
    int current_value;
    int passenger_id;

    while (1)
    {
        if (end_flag == 0)
        {
            if ((hall_count < max_limit))
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
                access_granted();

                // check bus semaphores
                if ((current_value = semctl(bus_sem_array_id, current_bus, GETVAL, 0)) == -1)
                {
                    perror("semctl: GETVAL");
                    exit(4);
                }

                printf(" current bus is: %d,  with avilable space = %d \n\n", current_bus, current_value);
                while (current_value > 0)
                {

                    passenger_id = deQueue(passengers);
                    sprintf(str_passenger_id, "%d", passenger_id);

                    to_bus_msg.mtype = BUS_MESSAGE_TYPE;
                    strcpy(to_bus_msg.mtext, str_passenger_id);

                    if (msgsnd(bus_msg_array[current_bus], &to_bus_msg, BUS_MESSAGE_SIZE, 0) == -1)
                    {
                        perror("Client1212: msgsend");
                        break;
                    }
                    printf("passenger %d got in bus %d\n", passenger_id, current_bus);

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

                        if (current_value2 == 0)
                        {
                            cyan();
                            printf("send signal to bus %d to move \n", current_bus);
                            fflush(stdout);
                            reset();
                            kill(bus_pid_array[current_bus], SIGUSR1);
                        }

                        break;
                    }
                }

                // if there is no space left in the bus -> go to next bus
                if (current_value == 0)
                {
                    current_bus = (current_bus + 1) % num_of_busses;
                }
            }
            else
            {

                while (hall_count > min_limit)
                {
                    if ((current_value = semctl(bus_sem_array_id, current_bus, GETVAL, 0)) == -1)
                    {
                        perror("semctl: GETVAL");
                        exit(4);
                    }

                    while (current_value > 0)
                    {

                        passenger_id = deQueue(passengers);
                        sprintf(str_passenger_id, "%d", passenger_id);

                        to_bus_msg.mtype = BUS_MESSAGE_TYPE;
                        strcpy(to_bus_msg.mtext, str_passenger_id);

                        if (msgsnd(bus_msg_array[current_bus], &to_bus_msg, BUS_MESSAGE_SIZE, 0) == -1)
                        {
                            perror("Client1212: msgsend");
                            break;
                        }
                        printf("passenger %d got in bus %d\n", passenger_id, current_bus);

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

                            if (current_value2 == 0)
                            {
                                cyan();
                                printf("send signal to bus %d to move \n", current_bus);
                                fflush(stdout);
                                reset();
                                kill(bus_pid_array[current_bus], SIGUSR1);
                            }

                            break;
                        }
                    }

                    // if there is no space left in the bus -> go to next bus
                    if (current_value == 0)
                    {
                        current_bus = (current_bus + 1) % num_of_busses;
                    }
                }

                start_min = 1;
            }
        }
    }

    return 0;
}

void cleanup()
{
    int passenger_id;
    while (1)
    {
        passenger_id = deQueue(passengers);
        // queue is empty
        if (passenger_id == -1)
        {
            msgctl(mid, IPC_RMID, (struct msqid_ds *)0);
            printf("HALL IS CLEAN NOW\n");
            fflush(stdout);
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

// SIGALRM Catcher
void signal_alarm_catcher(int the_sig)
{
    printf("Hall with pid = %d, exited from alarm\n", getpid());
    fflush(stdout);
    exit(1);
}

void access_granted()
{
    long semid;
    key_t key;

    key = ftok(".", SEM_ACCESS_GRANTED_SEED);
    if ((semid = semget(key, 1, IPC_EXCL | 0660)) == -1)
    {
        perror("cant access sem");
    }

    if (semop(semid, &acquire, 1) == -1)
    {
        perror("semop -- producer -- waiting for consumer to read number hall ");
        exit(3);
    }

    char *shmptr;
    int tmp;
    long shmid;

    if ((key = ftok(".", ACCESS_GRANTED_SEED)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((shmid = shmget(key, 10, 0666)) != -1)
    {
        if ((shmptr = (char *)shmat(shmid, 0, 0)) == (char *)-1)
        {
            perror("shmptr -- parent -- attach");
            exit(1);
        }
        tmp = atoi(shmptr);
        tmp++;
        sprintf(shmptr, "%d", tmp);
        printf("Access granted count is: ------------------------ (%s) ---------------------\n", shmptr);
        fflush(stdout);
        shmdt(shmid);
    }
    else
    {
        perror("shm attach");
    }

    if (semop(semid, &release, 1) == -1)
    {
        perror("semop -- producer -- indicating new number has been made");
        exit(5);
    }
    if (tmp == access_granted_count)
    {
        int parent = getppid();
        if (parent != 1)
            kill(getppid(), SIGUSR1);
    }
}

void signal_end_catcher(int the_sig)
{
    end_flag = 1;
    cleanup();
}
