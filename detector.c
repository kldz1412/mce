#include "detector.h"

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

int *getClock(char *str)
{
	int *clock, size, i;
	size = strlen(str) / 2;
	clock = (int *) malloc(size * sizeof(int));
	for (i = 0; i < size; i++)
	{
		clock[i] = str[i * 2 + 1] - '0';
	}
	return clock;
}

bool equalClock(int *clock1, int *clock2, int size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		if (clock1[i] != clock2[i])
		{
			return false;
		}
	}
	return true;
}

int main(int argc, char **argv)
{
	int size, i, index, *clock, *tmpClock, tmpInt, eventCode;
	char fileName[15];
	char *buffer, *clockStr, *tmpBuffer, *tmpStr;
	FILE **pFile;
	List *head;

	size = atoi(argv[1]);
	pFile = (FILE **) malloc(size * sizeof(FILE *));
	head = (List *) malloc(size * sizeof(List));
	for (i = 0; i < size; i++)
	{
		sprintf(fileName, "trace%d", i);
		pFile[i] = fopen(fileName, "r");
		if (pFile[i] == NULL)
		{
			perror("Error opening file");
		}

		buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
		fgets(buffer, BUFFER_SIZE, pFile[i]);
		tmpBuffer = strchr(buffer, '\t') + 1;
		memcpy(head[i].base, tmpBuffer, 10);
		tmpBuffer = strchr(tmpBuffer, '\t') + 1;
		tmpInt = strcspn(tmpBuffer, "\t");
		tmpStr = (char *) malloc(tmpInt * sizeof(char));
		memcpy(tmpStr, tmpBuffer, tmpInt);
		head[i].size = atoi(tmpStr);
		printf("%d\n", head[i].size);
		free(tmpStr);
		tmpBuffer = strchr(tmpBuffer, '\t') + 1;
		tmpInt = strcspn(tmpBuffer, "\t");
		tmpStr = (char *) malloc(tmpInt * sizeof(char));
		memcpy(tmpStr, tmpBuffer, tmpInt);
		head[i].disp_unit = atoi(tmpStr);
		free(tmpStr);
		free(buffer);
	}

	buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
	index = 0;
	while (index < size)
	{
		if (fgets(buffer, BUFFER_SIZE, pFile[index]) != NULL)
		{	
			if (getEventCode(buffer) == FENCE)
			{
				clockStr = (char *) malloc((size * 2 + 2) * sizeof(char));
				memcpy(clockStr, strstr(buffer, "["), size * 2 + 1);
				clockStr[size * 2 + 1] = '\0';
				clock = getClock(clockStr);
				
				for (i = index + 1; i < size; i++)
				{
					while (true)
					{
						if (fgets(buffer, BUFFER_SIZE, pFile[i]) != NULL)
						{
							if (getEventCode(buffer) == FENCE)
							{
								clockStr = (char *) malloc((size * 2 + 2) * sizeof(char));
                               					memcpy(clockStr, strstr(buffer, "["), size * 2 + 1);
                               					clockStr[size * 2 + 1] = '\0';
								if (equalClock(clock, getClock(clockStr), size) == true)
								{
									break;
								}
							}
						}
					}
				}
	
				for (i = index; i < size; i++)
                        	{
                                	if (fgets(buffer, BUFFER_SIZE, pFile[i]) != NULL)
                                	{
						eventCode = getEventCode(buffer);
                                        	if (eventCode != FENCE)
						{
							if (eventCode >= PUT && eventCode <= ACCUMULATE)
							{
								//head[i]->next
							}
						}
						else
						{
							break;
						}
                                	}
                        	}
			}
			
			else if (getEventCode(buffer) == 2)
			{
			}
			else { /*do nothing*/ }
		}
		else 
		{
			index++;
		}
	}

	for (i = 0; i < size; i++)
	{
		fclose(pFile[i]);
	}
	free(pFile);
	free(head);

	return 0;
}