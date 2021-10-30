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

#define SERVER 1L /* message for the server */
#define MSGSZ 128

typedef struct msgbuf {
	long mtype;
	char mtext[MSGSZ];
} MESSAGE;

#endif
