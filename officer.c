
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    printf("the officer id is: %d\n", getpid());

    return 0;
}