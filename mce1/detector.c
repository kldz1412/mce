#include "detector.h"

int numproc;

int getEventCode(char *str)
{
	if (strstr(str, "Fence") != NULL)
		return FENCE;
	if (strstr(str, "Barrier") != NULL)
		return BARRIER;
	if (strstr(str, "Put") != NULL)
		return PUT;
	if (strstr(str, "Get") != NULL)
		return GET;
	if (strstr(str, "Accumulate") != NULL)
		return ACCUMULATE;
	if (strstr(str, "Post") != NULL)
		return POST;
	if (strstr(str, "Start") != NULL)
		return START;
	if (strstr(str, "Complete") != NULL)
		return COMPLETE;
	if (strstr(str, "Wait") != NULL)
		return WAIT;
	if (strstr(str, "Lock") != NULL)
		return LOCK;
	if (strstr(str, "Unlock") != NULL)
		return UNLOCK;
	if (strstr(str, "Send") != NULL)
		return SEND;
	if (strstr(str, "Recv") != NULL)
		return RECV;
	if (strstr(str, "Load") != NULL)
		return LOAD;
	if (strstr(str, "Store") != NULL)
		return STORE;
	if (strstr(str, "Create") != NULL)
		return CREATE;
}

char *convertCode2Name(int code)
{
	char *tmp;
	switch (code)
	{
		case FENCE:
			tmp = malloc(6 * sizeof(char));
			strcpy(tmp, "FENCE\0");
			return tmp;
		case BARRIER:
                        tmp = malloc(8 * sizeof(char));
                        strcpy(tmp, "BARRIER\0");
                        return tmp;
		case GET:
                        tmp = malloc(4 * sizeof(char));
                        strcpy(tmp, "GET\0");
                        return tmp;
		case PUT:
                        tmp = malloc(4 * sizeof(char));
                        strcpy(tmp, "PUT\0");
                        return tmp;
		case ACCUMULATE:
                        tmp = malloc(11 * sizeof(char));
                        strcpy(tmp, "ACCUMULATE\0");
                        return tmp;
        	case POST:
                        tmp = malloc(5 * sizeof(char));
                        strcpy(tmp, "POST\0");
                        return tmp;
        	case START:
                        tmp = malloc(6 * sizeof(char));
                        strcpy(tmp, "START\0");
                        return tmp;
		case COMPLETE:
                        tmp = malloc(9 * sizeof(char));
                        strcpy(tmp, "COMPLETE\0");
                        return tmp;
		case WAIT:
                        tmp = malloc(5 * sizeof(char));
                        strcpy(tmp, "WAIT\0");
                        return tmp;
        	case LOCK:
                        tmp = malloc(5 * sizeof(char));
                        strcpy(tmp, "LOCK\0");
                        return tmp;
        	case UNLOCK:
                        tmp = malloc(7 * sizeof(char));
                        strcpy(tmp, "UNLOCK\0");
                        return tmp;
		case SEND:
                        tmp = malloc(5 * sizeof(char));
                        strcpy(tmp, "SEND\0");
                        return tmp;
		case RECV:
                        tmp = malloc(5 * sizeof(char));
                        strcpy(tmp, "RECV\0");
                        return tmp;
        	case LOAD:
                        tmp = malloc(5 * sizeof(char));
                        strcpy(tmp, "LOAD\0");
                        return tmp;
        	case STORE:
                        tmp = malloc(6 * sizeof(char));
                        strcpy(tmp, "STORE\0");
                        return tmp;
        	case CREATE:
                        tmp = malloc(7 * sizeof(char));
                        strcpy(tmp, "CREATE\0");
                        return tmp;
        }
}

int *getClock(char *buffer)
{
	char *tmpStr = (char *) malloc (BUFFER_SIZE);
	memcpy(tmpStr,buffer,strlen(buffer)+1);
	char *clockStr = strstr(tmpStr, "\t");
	clockStr++;
	int *clock, i = 0;
	clock = (int *) malloc(numproc * sizeof(int));
	char * p = strtok(clockStr, "|");
	while (p) {
		clock[i] = atoi(p);
		p = strtok(NULL, "|");
		i++;
	}
	//free clockStr
	return clock;
}


bool isConcurrent(int *clock1, int rank1, int *clock2, int rank2)
{
	if (clock1[rank2] >= clock2[rank2])
		return false;
	if (clock2[rank1] >= clock1[rank1])
		return false;
	return true;
}

bool isCommConflict(Comm *op1, int rank1, Comm *op2, int rank2) {
	if (op1->target_addr == op2->target_addr) {
		switch (op1->code) {
			case PUT:
				if (isConcurrent(op1->clock, rank1, op2->clock, rank2)) {
					return true;
				}
				break;
			case GET:
				if (op2->code == GET) {
					return false;
				}
				else if (isConcurrent(op1->clock, rank1, op2->clock, rank2)) {
					return true;
				}
				return false;
				break;
			case ACCUMULATE:
				if (op2->code == ACCUMULATE) {
					return false;
				}
				else if (isConcurrent(op1->clock, rank1, op2->clock, rank2)) {
					return true;
				}
				return false;
				break;
		}
	}
	else {
		return false;
	}
}

void checkNinsertComm(List **aList,char* buffer, int self_rank)
{
	char * p;
	p = strtok (buffer, "\t");
	p = strtok (NULL, "\t");
	p = strtok (NULL, "\t");
	p = strtok (NULL, "\t");
	int target_rank = atoi(p);
	if (aList[target_rank]->commHead == NULL)
	{
		aList[target_rank]->commTail = (Comm *) malloc(sizeof(Comm));
		aList[target_rank]->commHead = aList[target_rank]->commTail;
	}
	else
	{
		aList[target_rank]->commTail->next = (Comm *) malloc(sizeof(Comm));
		aList[target_rank]->commTail = aList[target_rank]->commTail->next;
	}
	aList[target_rank]->commTail->code = getEventCode(buffer);
	aList[target_rank]->commTail->target_addr = aList[target_rank]->base;
	aList[target_rank]->commTail->clock = aList[self_rank]->lastClock;
	aList[target_rank]->commTail->origin = self_rank;
	aList[target_rank]->commTail->next = NULL;
	//check
	Comm *tmp = aList[target_rank]->commHead;
	while (aList[target_rank]->commTail != tmp) {
		if (isCommConflict(tmp, tmp->origin, aList[target_rank]->commTail, self_rank)) {
			printf("MCE across process on %s between %s and %s in process %d\n", tmp->target_addr, 
            	convertCode2Name(tmp->code), convertCode2Name(aList[target_rank]->commTail->code), target_rank);
		}
		tmp = tmp->next;
	}
}

bool isConflictAcross(Loca *loca, Comm *comm) {
	if (comm->code == PUT || comm->code == ACCUMULATE) {
		if (strcmp(loca->varAddr, comm->target_addr) == 0) return true;
		return false;
	}
	else {
		if (loca->code == STORE || loca->code == GET) return true;
		return false;
	}
}

bool isConflictInside(Loca *op1, Loca *op2){
	if (op1->code == GET) {
		if (strcmp(op1->varAddr, op2->varAddr) == 0) { //overlapping address
			return true;
		}
	}
	else if (op1->code == PUT || op1->code == ACCUMULATE) {
		if (op2->code == STORE || op2->code == GET) {
			if (strcmp(op1->varAddr, op2->varAddr) == 0) { //overlapping address
			return true;
		}
		}
	}
	return false;
}

void checkNinsertLoca(List *aList,char* buffer)
{
	if (aList->locaGroupTail == NULL) insertGroup(aList, aList->lastSyn);
	if (aList->locaGroupTail->locaHead == NULL)
	{
        aList->locaGroupTail->locaTail = (Loca *) malloc(sizeof(Loca));
        aList->locaGroupTail->locaHead = aList->locaGroupTail->locaTail;
    }
    else
    {
        aList->locaGroupTail->locaTail->next = (Loca *) malloc(sizeof(Loca));
        aList->locaGroupTail->locaTail = aList->locaGroupTail->locaTail->next;
    }
	aList->locaGroupTail->locaTail->code = getEventCode(buffer);
	char *varAddr = strstr(buffer, "\t");
	varAddr++;
	aList->locaGroupTail->locaTail->varAddr = varAddr;
	aList->locaGroupTail->locaTail->next = NULL;
	//check
	Loca *tmp = aList->locaGroupTail->locaHead;
	while (aList->locaGroupTail->locaTail != tmp) {
		if (isConflictInside(tmp, aList->locaGroupTail->locaTail)) {
			printf("MCE within an epoch on %s between %s and %s in process %d\n", tmp->varAddr, 
            	convertCode2Name(tmp->code), convertCode2Name(aList->locaGroupTail->locaTail->code), aList->rank);
		}
		tmp = tmp->next;
	}
}



void insertGroup(List *aList,char* buffer)
{
	if (aList->locaGroupTail == NULL)
	{
        aList->locaGroupTail = (LocaGroup *) malloc(sizeof(LocaGroup));
        aList->locaGroupHead = aList->locaGroupTail;
    }
    else
    {
        aList->locaGroupTail->next = (LocaGroup *) malloc(sizeof(LocaGroup));
        aList->locaGroupTail = aList->locaGroupTail->next;
    }
	aList->locaGroupTail->clock = aList->lastClock;
	aList->locaGroupTail->next = NULL;
	aList->locaGroupTail->locaHead = NULL;
	aList->locaGroupTail->locaTail = NULL;
}



bool isSynOp(int code){
	if (code < 6) return false;
	if (code > 11) return false;
	return true;
}

char *getData(char **buffer)
{
	char *tmpBuffer, *tmpStr;
	int tmpInt;
	
	tmpBuffer = *buffer;
	*buffer = strchr(*buffer, '\t') + 1;
        tmpInt = strcspn(tmpBuffer, "\t\n");
        tmpStr = (char *) malloc((tmpInt + 1) * sizeof(char));
        memcpy(tmpStr, tmpBuffer, tmpInt);
	tmpStr[tmpInt] = '\0';

	return tmpStr;
}

int main(int argc, char **argv)
{
	int size, i, index, *clock, *tmpClock, tmpInt, eventCode, j;
	char fileName[15];
	char *buffer, *clockStr, *tmpBuffer, *tmpStr;

	size = atoi(argv[1]);
	numproc = atoi(argv[1]);
	printf("1\n");
	FILE **pFile = (FILE **) malloc(size * sizeof(FILE *));
	List **aList = (List **) malloc(size * sizeof(List *));
	printf("2\n");
	for (i = 0; i < size; i++)
	{
		sprintf(fileName, "trace%d", i);
		pFile[i] = fopen(fileName, "r");
		if (pFile[i] == NULL)
		{
			perror("Error opening file");
		}
		aList[i] = (List *) malloc(sizeof (List)); 
		buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
		fgets(buffer, BUFFER_SIZE, pFile[i]);

		tmpBuffer = buffer;
                tmpStr = getData(&tmpBuffer);
		free(tmpStr);

		tmpStr = getData(&tmpBuffer);
		aList[i]->base = tmpStr;

		tmpStr = getData(&tmpBuffer);
		aList[i]->size = atoi(tmpStr);
		free(tmpStr);

		tmpStr = getData(&tmpBuffer);
		aList[i]->disp_unit = atoi(tmpStr);
		free(tmpStr);

		aList[i]->commHead = NULL;
		aList[i]->commTail = NULL;
		aList[i]->locaGroupHead = NULL;
		aList[i]->locaGroupTail = NULL;
		aList[i]->lastSyn = (char *) malloc (BUFFER_SIZE);
		free(buffer);
	}
	printf("2\n");
	bool isFinished = true;
	while (true) {
		for (i = 0; i < numproc; i++){
			while (fgets(buffer, BUFFER_SIZE, pFile[i]) != NULL) {
				printf("%s\n",buffer);
				int code = getEventCode(buffer);
				if (code == FENCE || code == BARRIER) {
					aList[i]->lastClock = getClock(buffer);
					printf("chua break\n" );
					isFinished = false;
					strcpy(aList[i]->lastSyn, buffer);
					break;
				}
				else {
					if (isSynOp(code)) {
						aList[i]->lastClock = getClock(buffer);
						//aList[i]->lastSyn = (char *) malloc (BUFFER_SIZE);
					strcpy(aList[i]->lastSyn, buffer);
						insertGroup(aList[i], buffer);
					}
					else {
						if (code >= GET && code <= ACCUMULATE) {
							checkNinsertLoca(aList[i], buffer);
							checkNinsertComm(aList, buffer, i);
						}
						else if (code >= LOAD && code <= STORE) {
							checkNinsertLoca(aList[i], buffer);
						}
					}
				}
			}
			if (aList[i]->locaGroupHead != NULL || aList[i]->commHead != NULL) isFinished = false;
		}
		printf("1\n");
		if (isFinished) break;
		//check across
		printf("1\n");
		//checkAcross(aList);
		for (i = 0; i< numproc; i++){
			//if (!aList[i]->commHead) continue;
			Comm *tmpComm = aList[i]->commHead;
			while (tmpComm != NULL) {
				LocaGroup *tmpGroup = aList[i]->locaGroupHead;
				while (!tmpGroup) {
					if (isConcurrent(tmpComm->clock, tmpComm->origin, tmpGroup->clock, aList[i]->rank)) {
					Loca *tmpLoca = tmpGroup->locaHead;
					while (!tmpLoca) {
						//check tmpLoca vs tmpComm
						if (isConflictAcross(tmpLoca, tmpComm)) {
							printf("MCE across process on %s between %s and %s in process %d\n", tmpLoca->varAddr, 
	            				convertCode2Name(tmpLoca->code), convertCode2Name(tmpComm->code), i);
						}
						tmpLoca = tmpLoca->next;
					}
					tmpGroup = tmpGroup->next;
				}
				else {
					break;
				}
				}
				tmpComm = tmpComm->next;
			}
		}
		printf("3\n");
		for (i = 0; i < numproc; i++) {
			aList[i]->commHead = 0;
			aList[i]->commTail = 0;
			aList[i]->locaGroupHead = 0;
			aList[i]->locaGroupTail = 0;
		}
	}
	for (i = 0; i < numproc; i++) {
			fclose(pFile[i]);
		}
	printf("1\n");
	return 0;
}
