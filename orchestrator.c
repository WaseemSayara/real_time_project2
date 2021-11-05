#include "local.h"

void signal_int_catcher(int);
void signal_usr1_catcher(int);
void signal_usr2_catcher(int);
void bus_semaphore(int, int);
void create_shared_memory(char);
void create_semaphore(char);

static ushort start_val[2] = {1, 0};
int semid, producer = 0, i, n, p_sleep, c_sleep;
union semun arg;

int end_flag = 0;

int main(int argc, char *argv[])
{
    int i, status, num_of_busses, capacity_of_bus,
        num_of_officers, hall_max_count, hall_min_count, sem_value,
        access_granted, access_denied, impatient;
    long shmid_1, shmid_2, shmid_3, sem_array_id;
    char tmp[20], seed = 'A', *shmptr1, *shmptr2, *shmptr3, *tmp_char;
    pid_t pid, hall, pid_array[3];
    key_t key, sem_array_key;
    FILE *variables;

    if (sigset(SIGUSR1, signal_usr1_catcher) == SIGUSR1)
    {
        perror("Sigset can not set SIGINT");
        exit(SIGUSR1);
    }
    if (sigset(SIGUSR2, signal_usr1_catcher) == SIGUSR2)
    {
        perror("Sigset can not set SIGINT");
        exit(SIGUSR1);
    }
    if (sigset(SIGINT, signal_int_catcher) == SIGINT)
    {
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }

    variables = fopen("variables.txt", "r");
    fscanf(variables, "%s %d\n", &tmp, &num_of_busses);
    fscanf(variables, "%s %d\n", &tmp, &capacity_of_bus);
    fscanf(variables, "%s %d\n", &tmp, &num_of_officers);
    fscanf(variables, "%s %d\n", &tmp, &hall_max_count);
    fscanf(variables, "%s %d\n", &tmp, &hall_min_count);
    fscanf(variables, "%s %d\n", &tmp, &access_granted);
    fscanf(variables, "%s %d\n", &tmp, &access_denied);
    fscanf(variables, "%s %d\n", &tmp, &impatient);
    fclose(variables);

    /*
* Create & initialize bus semaphore
*/

    bus_semaphore(num_of_busses, capacity_of_bus);
    create_semaphore(SEM_ACCESS_GRANTED_SEED);

    /*
* Create shared memory
*/
    create_shared_memory(ACCESS_GRANTED_SEED);
    create_shared_memory(ACCESS_DENIED_SEED);
    create_shared_memory(IMPATIENT_SEED);

    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------

    pid_t bus_array[num_of_busses];
    pid_t officer_array[num_of_officers];

    for (i = 0; i < num_of_busses; i++)
    {
        pid = fork(); //store forked proceee pid in pid variable
        // Failed fork process
        if (pid == -1)
        {
            perror("failed to fork bus");
            exit(2);
        }
        // The process is a child
        else if (pid == 0)
        {

            char str_bus_num[3];
            sprintf(str_bus_num, "%d", i);
            // Assign Referee process
            if (execl("./bus", str_bus_num, (char *)0) == -1)
            {
                perror("Faild to execute ./bus!");
                exit(3);
            }
        }
        else
        {

            bus_array[i] = pid;
        }
    }
    sleep(1);

    for (i = 0; i < num_of_officers; i++, seed++)
    {
        pid = fork(); //store forked proceee pid in pid variable
        // Failed fork process
        if (pid == -1)
        {
            perror("failed to fork officer");
            exit(2);
        }
        // The process is a child
        else if (pid == 0)
        {
            // Assign Referee process
            char str_access_denied[5];
            sprintf(str_access_denied, "%d", access_denied);
            if (execl("./officer", &seed, str_access_denied, (char *)0) == -1)
            {
                perror("Faild to execute ./officer!");
                exit(3);
            }
        }
        else
        {
            officer_array[i] = pid;
        }
    }
    sleep(1);

    int size = num_of_busses * 8;
    char concatenate_pids[size];
    char temp[8];

    // Concatenate first pid in the list
    sprintf(concatenate_pids, "%d", bus_array[0]);

    for (i = 1; i < num_of_busses; i++)
    {
        sprintf(temp, "%d", bus_array[i]);
        strcat(concatenate_pids, "-");
        strcat(concatenate_pids, temp);
    }

    pid = fork(); //store forked proceee pid in pid variable
    // Failed fork process
    if (pid == -1)
    {
        perror("failed to fork hall");
        exit(2);
    }
    // The process is a child
    else if (pid == 0)
    {
        // Assign Referee process

        char str_max[5], str_min[5];
        sprintf(str_max, "%d", hall_max_count);
        sprintf(str_min, "%d", hall_min_count);

        char str_access_granted[5];
        sprintf(str_access_granted, "%d", access_granted);

        printf(" 000000000000 %s\n", concatenate_pids);

        if (execl("./hall", str_max, str_min, concatenate_pids, str_access_granted, (char *)0) == -1)
        {
            perror("Faild to execute ./hall!");
            exit(3);
        }
    }
    else
    {
        hall = pid;
    }

    sleep(1);
    srand(getpid());

    //????????????????????????????????????????????????????? WHY K is constant from 0 to 4 & K express what!! ???????????????????????????????????????????????????????????
    int k = 0;
    while (k < 15)
    {
        int sleep_count, passengers_count;
        sleep_count = (rand() % 3) + 1;
        passengers_count = (rand() % 6) + 5;
        char str_impatient[5];
        sprintf(str_impatient, "%d", impatient);

        for (i = 0; i < passengers_count; i++)
        {

            if (end_flag == 1)
            {
                break;
            }

            pid = fork();

            if (pid == -1)
            {

                perror("failed to fork passenger");
                exit(2);
            }
            // The process is a child
            else if (pid == 0)
            {
                // Assign Referee process

                char str_officers_count[4];
                sprintf(str_officers_count, "%d", num_of_officers);

                if (execl("./passenger", str_officers_count, str_impatient, (char *)0) == -1)
                {
                    perror("Faild to execute ./passenger!");
                    exit(3);
                }
            }
        }
        sleep(sleep_count);
        k++;
    }

    //TODO: clean the program.

    printf(" letss end \n");

    return 0;
}

/*
*
*
// ##################################################### Bus Semaphore ##################################################
*
*
*/

void create_semaphore(char seed)
{
    long semid;
    key_t key;

    key = ftok(".", seed);
    if ((semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0660)) != -1)
    {

        if (semctl(semid, 0, SETVAL, 1) == -1)
        {
            perror("ERROR in SETVAL");
        }
    }
}

void bus_semaphore(int num_of_busses, int capacity_of_bus)
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
    if ((sem_array_id = semget(sem_array_key, num_of_busses, IPC_CREAT | 0660)) == -1)
    {
        perror("semget: IPC_CREAT | 0660");
        exit(1);
    }

    printf("Semaphore identifier %d\n", sem_array_id);
    fflush(stdout);

    // Initailize semaphore cell values

    struct semid_ds sem_buf;
    ushort sem_array[num_of_busses];
    union semun arg;

    for (int i = 0; i < num_of_busses; i++)
    {
        sem_array[i] = capacity_of_bus;
    }

    /*
* Set arg (the union) to the addr of the storage location
* for returned semid_ds values.
*/
    arg.buf = &sem_buf;

    /*
* Set arg (the union) to the addr of the initializing
* vector.
*/

    //Initalize semaphore
    arg.array = sem_array;
    if (semctl(sem_array_id, 0, SETALL, arg) == -1)
    {
        perror("semctl: SETALL 1212");
        exit(3);
    }

    for (int i = 0; i < num_of_busses; i++)
    { /* display contents */
        if ((sem_value = semctl(sem_array_id, i, GETVAL, 0)) == -1)
        {
            perror("semctl: GETVAL");
            exit(4);
        }

        printf("Semaphore %d has value of %d\n", i, sem_value);
        fflush(stdout);
    }

    // // Remove Semaphore

    // if (semctl(sem_array_id, 0, IPC_RMID, 0) == -1)
    // {
    //     perror("semctl: IPC_RMID"); /* remove semaphore */
    //     exit(5);
    // }
}

/*
*
*
// ##################################################### Shared Memory ##################################################
*
*
*/

void create_shared_memory(char proj_id)
{
    key_t key;
    long shmid;
    char *shmptr, *tmp_char;

    if ((key = ftok(".", proj_id)) == -1)
    {
        perror("Client: key generation");
        return 1;
    }

    if ((shmid = shmget(key, 10, IPC_CREAT | 0666)) != -1)
    {

        if ((shmptr = (char *)shmat(shmid, 0, 0)) == (char *)-1)
        {
            perror("shmptr -- parent -- attach");
            exit(1);
        }

        tmp_char = shmptr;
        *tmp_char++ = '0';
        *tmp_char = NULL;
    }
    else
    {
        perror("shmid create ");
        exit(2);
    }

    shmdt(shmid);
}

/*
*
*
// ##################################################### Signal Catcher ##################################################
*
*
*/

void signal_usr1_catcher(int the_sig)
{
    printf("\n Program ended due to enough passengers were GRANTED to access the porders \n");
    fflush(stdout);
    end_flag = 1;
}

void signal_usr2_catcher(int the_sig)
{
    printf("\n Program ended due to enough passengers were DENIED to access the porders \n");
    fflush(stdout);
    end_flag = 1;
}

void signal_int_catcher(int the_sig)
{
    printf("\n Program ended due to enough passengers were IMPATIENT and left \n");
    fflush(stdout);
    end_flag = 1;
}