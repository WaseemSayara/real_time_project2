
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    printf("the hall id is: %d\n", getpid());

    return 0;
}