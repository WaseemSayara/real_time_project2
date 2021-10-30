
#include "local.h"

void signal_quit_catcher(int);

int main(int argc, char *argv[])
{
    int i, status, num_of_busses, num_of_officers;
    pid_t pid, hall, pid_array[3];
    static char message[BUFSIZ];
    char tmp[20];
    char seed = 'A';
    char *seed_p;
    FILE *variables;

    // if (sigset(SIGSTOP, signal_quit_catcher) == SIGSTOP)
    // {
    //     perror("Sigset can not set SIGINT");
    //     exit(SIGSTOP);
    // }
    // if (sigset(SIGINT, signal_quit_catcher) == SIGINT)
    // {
    //     perror("Sigset can not set SIGINT");
    //     exit(SIGINT);
    // }

    variables = fopen("variables.txt", "r");

    fscanf(variables, "%s %d\n", &tmp, &num_of_busses);
    fscanf(variables, "%s %d\n", &tmp, &num_of_officers);
    fclose(variables);

    printf("%d  %d\n", num_of_busses, num_of_officers);

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

            int status = execl("./bus", (char *)0);

            if (status == -1)
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
            int status = execl("./officer", &seed, (char *)0);

            if (status == -1)
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
        perror("failed to fork officer");
        exit(2);
    }
    // The process is a child
    else if (pid == 0)
    {
        // Assign Referee process

        int status = execl("./hall", (char *)0);

        if (status == -1)
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

    int k=0;
    while (k <10)
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
                char *str_officers_count;
                sprintf(str_officers_count, "%d", num_of_officers);

                int status = execl("./passenger", str_officers_count, (char *)0);

                if (status == -1)
                {
                    perror("Faild to execute ./passenger!");
                    exit(3);
                }
            }
        sleep(sleep_count);
        }
        k++;
    }

    // for (i = 0; i < 3; i++)
    // {
    //     pid = fork(); //store forked proceee pid in pid variable

    //     // Failed fork process
    //     if (pid == -1)
    //     {
    //         perror("failed to fork childs");
    //         exit(2);
    //     }
    //     // The process is a child
    //     else if (pid == 0)
    //     {
    //         // Assign Referee process
    //         if (i == 0)
    //         {
    //             char pipe_read[5], pipe_write[5];

    //             // Convert from integer to String to send in argv
    //             sprintf(pipe_read, "%d", f_des[0]);
    //             sprintf(pipe_write, "%d", f_des[1]);

    //             int status = execl("./referee", pipe_read, pipe_write, (char *)0);

    //             if (status == -1)
    //             {
    //                 perror("Faild to execute ./referee!");
    //                 exit(3);
    //             }
    //         }

    //         // Assign children processes
    //         else
    //         {
    //             // Used to identified children ( 1 or 2)
    //             char child_num[3];
    //             sprintf(child_num, "%d", i);
    //             int status = execl("./child", child_num, (char *)0);

    //             if (status == -1)
    //             {
    //                 perror("Faild to execute ./child!");
    //                 exit(4);
    //             }
    //         }
    //     }

    //     // Parent Case
    //     else
    //     {
    //         // save children pids
    //         pid_array[i] = pid;
    //     }
    // }

    return 0;
}

void signal_quit_catcher(int the_sig)
{

    exit(1);
}