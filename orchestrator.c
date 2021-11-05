#include "local.h"

void signal_quit_catcher(int);
void bus_semaphore(int, int);
void create_shared_memory(int);

int main(int argc, char *argv[])
{
    int i, status, num_of_busses, capacity_of_bus, num_of_officers, hall_max_count, hall_min_count, sem_value;
    long shmid_1, shmid_2, shmid_3, sem_array_id;
    char tmp[20], seed = 'A', *shmptr1, *shmptr2, *shmptr3, *tmp_char;
    pid_t pid, hall, pid_array[3];
    key_t key, sem_array_key;
    FILE *variables;

    // if (sigset(SIGSTOP, signal_quit_catcher) == SIGSTOP)
    // {
    // perror("Sigset can not set SIGINT");
    // exit(SIGSTOP);
    // }
    // if (sigset(SIGINT, signal_quit_catcher) == SIGINT)
    // {
    // perror("Sigset can not set SIGINT");
    // exit(SIGINT);
    // }

    variables = fopen("variables.txt", "r");
    fscanf(variables, "%s %d\n", &tmp, &num_of_busses);
    fscanf(variables, "%s %d\n", &tmp, &capacity_of_bus);
    fscanf(variables, "%s %d\n", &tmp, &num_of_officers);
    fscanf(variables, "%s %d\n", &tmp, &hall_max_count);
    fscanf(variables, "%s %d\n", &tmp, &hall_min_count);
    fclose(variables);

    /*
* Create & initialize bus semaphore
*/

    bus_semaphore(num_of_busses, capacity_of_bus);

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
            // Assign Referee process
            if (execl("./bus", (char *)0) == -1)
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
            if (execl("./officer", &seed, (char *)0) == -1)
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

        if (execl("./hall", str_max, str_min, (char *)0) == -1)
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
    while (k < 4)
    {
        int sleep_count, passengers_count;
        sleep_count = (rand() % 5) + 1;
        passengers_count = (rand() % 6) + 1;

        for (i = 0; i < passengers_count; i++)
        {

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

                if (execl("./passenger", str_officers_count, (char *)0) == -1)
                {
                    perror("Faild to execute ./passenger!");
                    exit(3);
                }
            }
        }
        sleep(sleep_count);
        k++;
    }

    // for (i = 0; i < 3; i++)
    // {
    // pid = fork(); //store forked proceee pid in pid variable

    // // Failed fork process
    // if (pid == -1)
    // {
    // perror("failed to fork childs");
    // exit(2);
    // }
    // // The process is a child
    // else if (pid == 0)
    // {
    // // Assign Referee process
    // if (i == 0)
    // {
    // char pipe_read[5], pipe_write[5];

    // // Convert from integer to String to send in argv
    // sprintf(pipe_read, "%d", f_des[0]);
    // sprintf(pipe_write, "%d", f_des[1]);

    // int status = execl("./referee", pipe_read, pipe_write, (char *)0);

    // if (status == -1)
    // {
    // perror("Faild to execute ./referee!");
    // exit(3);
    // }
    // }

    // // Assign children processes
    // else
    // {
    // // Used to identified children ( 1 or 2)
    // char child_num[3];
    // sprintf(child_num, "%d", i);
    // int status = execl("./child", child_num, (char *)0);

    // if (status == -1)
    // {
    // perror("Faild to execute ./child!");
    // exit(4);
    // }
    // }
    // }

    // // Parent Case
    // else
    // {
    // // save children pids
    // pid_array[i] = pid;
    // }
    // }

    return 0;
}

/*
*
*
// ##################################################### Bus Semaphore ##################################################
*
*
*/

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
        perror("semctl: SETALL");
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

    // Remove Semaphore

    if (semctl(sem_array_id, 0, IPC_RMID, 0) == -1)
    {
        perror("semctl: IPC_RMID"); /* remove semaphore */
        exit(5);
    }
}

/*
*
*
// ##################################################### Shared Memory ##################################################
*
*
*/

void create_shared_memory(int proj_id)
{
    key_t key;
    long shmid;
    char *shmptr, *tmp_char;
    char shared_memory_owner[25];

    sprintf(shared_memory_owner, "%d", proj_id);

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
        perror(shared_memory_owner);
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

void signal_quit_catcher(int the_sig)
{
    exit(1);
}