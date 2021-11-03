#ifndef __LOCAL_H__
#define __LOCAL_H__

/*
 * Common header file for Message Queue Example
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <semaphore.h>

#define ACCESS_GRANTED_SEED 1
#define ACCESS_DENIED_SEED 2
#define IMPATIENT_SEED 3
#define SEM_ARRAY_SEED 10

#define MSGSZ 12
#define ARAB_PERCENTS 0.7
#define HALL_SEED '$'
#define HALL_MESSAGE_TYPE 2
#define HALL_MESSAGE_SIZE 10

#define BUS_MESSAGE_TYPE 3
#define BUS_MESSAGE_SIZE 10



typedef struct msgbuf {
	long mtype;
	char mtext[MSGSZ];
} MESSAGE;

union semun
{
int val;
struct semid_ds *buf;
ushort *array;
};


#endif
