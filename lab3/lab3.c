#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char* dateStr;
char* curReqStr;
char* statusStr;


typedef struct Request {
	int dateReq;
	int requestNumber;
	struct Request* next;
} Request;


typedef struct {
    	Request* head, *tail;
} Queue;


void enqueue(Queue* queue, int timeOfRequest, int requestNumber) {
    Request* curReq = (Request*)malloc(sizeof(Request));
    curReq->next = NULL;
    curReq->dateReq = timeOfRequest;
    curReq->requestNumber = requestNumber;
    if (queue->tail == NULL) {
        queue->head = queue->tail = curReq;
        return;
    }
    queue->tail->next = curReq;
    queue->tail = curReq;
}


void dequeue(Queue* queue){
    if (queue->head == NULL)
        return;
    Request* curReq = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL)
        queue->tail = NULL;
    free(curReq);
}


int parseDate(char* dateStr) {
	struct tm timeOfCurRequest;

	timeOfCurRequest.tm_mday = atoi(strtok(dateStr, "/"));

	char* strOfMonth = strtok(NULL, "/");

	if (strcmp("Jan", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 0;
	else if (strcmp("Feb", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 1;
	else if (strcmp("Mar", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 2;
	else if (strcmp("Apr", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 3;
	else if (strcmp("May", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 4;
	else if (strcmp("Jun", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 5;
	else if (strcmp("Jul", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 6;
	else if (strcmp("Aug", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 7;
	else if (strcmp("Sep", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 8;
	else if (strcmp("Oct", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 9;
	else if (strcmp("Nov", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 10;
	else if (strcmp("Dec", strOfMonth) == 0)
		timeOfCurRequest.tm_mon = 11;

	timeOfCurRequest.tm_year = - 1900 + atoi(strtok(NULL, ":"));
	timeOfCurRequest.tm_hour = atoi(strtok(NULL, ":"));
	timeOfCurRequest.tm_min = atoi(strtok(NULL, ":"));
	timeOfCurRequest.tm_sec = atoi(strtok(NULL, " "));

	return mktime(&timeOfCurRequest);
}


char* parseString(FILE* logFile, int* ifLast) {
	char character;
	char* parsingStr = (char*) malloc(1000 * sizeof(char));
	int strLength = 0;
	while (1) {
		character = getc(logFile);
		if (character != EOF && character != '\n') {
			parsingStr[strLength] = character;
		} else {
			*ifLast = EOF == character;
			parsingStr[strLength] = '\0';
			break;
		}
		++strLength;
	}

	strtok(parsingStr, "[");
	dateStr = strtok(NULL, "]");
	strtok(NULL, "\"");
	curReqStr = strtok(NULL, "\"");
	statusStr = strtok(NULL, " ");
	return parsingStr;
}


void printIf1(FILE* inputFile) {
    int failedReq = 0;
    int ifLast = 0;
    while(!ifLast) {
	char* stringTemp = parseString(inputFile, &ifLast);
	if (statusStr != NULL)
            if (*statusStr == '5')
                printf("\n%d. %s", ++failedReq, curReqStr);
	free(stringTemp);
	}
	printf("\n\nCount of all failed requests: %d", failedReq);
}


void printIf2(FILE* inputFile, Queue* reqs) {
    int ifLast = 0;
    int maxCount = -1;
    int beginOfPeriod;

    unsigned long long needPeriod;
	printf("Please write need period in seconds: ");
	scanf("%llu", &needPeriod);
	if (needPeriod < 0) {
        printf("Error! Wrong period\n");
        exit(1);
	}

	for (int lineNum = 0; !ifLast; ++lineNum) {
		char* tmp = parseString(inputFile, &ifLast);
		if (dateStr != NULL) {
			int dateReqSec = parseDate(dateStr);
			enqueue(reqs, dateReqSec, lineNum);
			while (dateReqSec - reqs->head->dateReq > needPeriod)
				dequeue(reqs);
			int reqsCount = lineNum - reqs->head->requestNumber + 1;
			if (reqsCount > maxCount) {
				beginOfPeriod = reqs->head->dateReq;
				maxCount = reqsCount;
			}
		}
		free(tmp);
	}
	printf("\nThe largest number of requests: %d\n", maxCount);
	time_t period = beginOfPeriod;
	printf("In period from: %s\n", ctime(&period));
}


int main() {
	FILE* logFile;
	logFile = fopen("access_log_Jul95.txt", "r");

	Queue* reqs = (Queue*)calloc(1, sizeof(Queue));
    	reqs->head = reqs->tail = NULL;

	int numberOfTask;
	printf("Please write the number of the task: ");
	scanf("%d", &numberOfTask);
	if (numberOfTask == 1)
        	printIf1(logFile);
    	else
        	printIf2(logFile, reqs);
    	fclose(logFile);
	return 0;
}
