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
    char* tmp;
	struct tm timeOfCurRequest;

	timeOfCurRequest.tm_mday = atoi(strtok_r(dateStr, "/", &tmp));

	char* strOfMonth = strtok_r(NULL, "/", &tmp);
	char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	for (int monthsCount = 0; monthsCount < 12; ++monthsCount)
		if (strcmp(months[monthsCount], strOfMonth) == 0)
			timeOfCurRequest.tm_mon = monthsCount;

	timeOfCurRequest.tm_year = - 1900 + atoi(strtok_r(NULL, ":", &tmp));
	timeOfCurRequest.tm_hour = atoi(strtok_r(NULL, ":", &tmp));
	timeOfCurRequest.tm_min = atoi(strtok_r(NULL, ":", &tmp));
	timeOfCurRequest.tm_sec = atoi(strtok_r(NULL, " ", &tmp));

	return mktime(&timeOfCurRequest);
}


char* scanStr(FILE* logFile, int strLength, int* ifLast) {
	int character = getc(logFile);
	char* parsingStr;

	if (character == EOF || character == '\n') {
        parsingStr = (char*)malloc(strLength + 1);
		if (!parsingStr) {
            printf("We don't have enough memory");
            exit(1);
		}
		*ifLast = EOF == character;
		parsingStr[strLength] = '\0';
	} else {
		parsingStr = scanStr(logFile, strLength + 1, ifLast);
		parsingStr[strLength] = character;
	}

	return parsingStr;
}


char* parseString(FILE* logFile, int* ifLast) {
    char* tmp;
	char* parsingStr = scanStr(logFile, 0, ifLast);

	strtok_r(parsingStr, "[", &tmp);
	dateStr = strtok_r(NULL, "]", &tmp);
	strtok_r(NULL, "\"", &tmp);
	curReqStr = strtok_r(NULL, "\"", &tmp);
	statusStr = strtok_r(NULL, " ", &tmp);
	return parsingStr;
}


void printIf1(FILE* inputFile) {
    int failedReq = 0;
    int ifLast = 0;
    while(!ifLast) {
		char* stringTemp = parseString(inputFile, &ifLast);
		if (statusStr != NULL)
            if (*statusStr == '5')
                printf("\n%d. %s", ++failedReq, curReqStr);;
		free(stringTemp);
	}
	printf("\n\nCount of all failed requests: %d", failedReq);
}


void printIf2(FILE* inputFile, Queue* reqs) {
    int ifLast = 0;
    int maxCount = -1;
    int beginOfPeriod, endOfPeriod;

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
				endOfPeriod = dateReqSec;
				maxCount = reqsCount;
			}
		}
		free(tmp);
	}
	printf("\nThe largest amount of requests: %d\n", maxCount);
	time_t period = beginOfPeriod;
	printf("\nIn period from: %s", asctime(localtime(&period)));
	period = endOfPeriod;
	printf("To: %s\n", asctime(localtime(&period)));
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
