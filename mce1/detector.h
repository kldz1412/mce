#ifndef DETECTOR_H
#define DETECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define	BUFFER_SIZE		30

#define	FENCE			1
#define	BARRIER			2
#define	GET				3
#define	PUT				4
#define	ACCUMULATE		5
#define	POST			6
#define	START			7
#define	COMPLETE		8
#define	WAIT			9
#define	LOCK			10
#define	UNLOCK			11
#define	LOAD			12
#define	STORE			13
#define CREATE			14
#define SEND 			15
#define RECV			16

typedef struct Comm {
    int code;
	char *target_addr;
	int *clock;
	int origin;
    struct Comm *next;
} Comm;

typedef struct Loca {
    int code;
    char *varAddr;
    struct Loca *next;
} Loca;

/*typedef struct Unif {
	int code;
	char *varAddr;
	struct Unif *next;
} Unif;*/

typedef struct LocaGroup {
	struct Loca *locaHead;
	struct Loca *locaTail;
	struct LocaGroup *next;
	int *clock;
} LocaGroup;

typedef struct List {
    char *base;
    int size;
    int disp_unit;
    int rank;
    struct Comm *commHead;
	struct Comm *commTail;
    struct LocaGroup *locaGroupHead;
	struct LocaGroup *locaGroupTail;
	int *lastClock;
} List;

/*typedef struct Chai {
	int rank;
	struct Unif *head;
	struct Unif *tail;
} Chai;*/

int getEventCode(char *);
char *convertCode2Name(int);
int *getClock(char *);
bool isConcurrent(int *, int, int *, int);

void freeList(List *, int);
void printList(List *, int);

#endif
