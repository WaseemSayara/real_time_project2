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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define ACCESS_GRANTED 1
#define ACCESS_DENIED 2
#define IMPATIENT 3

#define MSGSZ 12
#define ARAB_PERCENTS 0.7
#define HALL_SEED '$'
#define HALL_MESSAGE_TYPE 2
#define HALL_MESSAGE_SIZE 10


typedef struct msgbuf {
	long mtype;
	char mtext[MSGSZ];
} MESSAGE;


#endif
