
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include <pthread.h>

#include <time.h>

#include <semaphore.h>
#include <string.h>

#include "room_lock.c"

// Supplied parameters
unsigned int numTeams; 
unsigned int numPirates;
unsigned int numNinjas;

unsigned int money;


// Holds the TID (Thread ID)
pthread_t *pirates;
pthread_t *ninjas;

// Functions
void makeThreads();

void printQueues();


//use a linked list to maintain the detail of each entry
struct Visit {
	unsigned int entityID;

	unsigned int entryTime;
	unsigned int exitTime;
	
	//... mas aqui luego
};


struct Queue { 
    struct ArrivalNode *front, *rear; 
}; 


unsigned int isNinja(int);
unsigned int start;


struct ArrivalNode *pirateHead;
struct ArrivalNode *ninjaHead;


sem_t ninjaSem;
sem_t pirateSem;

pthread_mutex_t teamLock;


pthread_mutex_t queueLock;
pthread_mutex_t fittedLock;
pthread_mutex_t *costumeLock;

pthread_mutex_t withhold_pirates;

pthread_cond_t ninjaOnlyCondition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t accessLock;

void addToQueue(struct Queue *, unsigned int, unsigned int);
struct ArrivalNode* popHead(struct Queue *);

void getFitted(int);

struct Queue pirateQueue;
struct Queue ninjaQueue;

unsigned int openRoom();
void decide();
unsigned int countQueueSize(struct Queue*);

unsigned long gameClock;
unsigned int earnings = 0;
unsigned int ninjaTurn;

void room_lock_and_wait(room_lock_t*, struct ArrivalNode*, enum Team);

room_lock_t *roomLock;

void *arrive(void *vargp) {
	int arrival_time = rand() % 24;
	
	while(!start);
	
	sleep(arrival_time);
	
	int thread_id = *((int *) vargp);
	
	pthread_mutex_lock(&queueLock);
	
		if(isNinja(thread_id)) {
				printf("A ninja arrived at %d hours\n", arrival_time);
				
				addToQueue(&ninjaQueue, thread_id, arrival_time);

				
		} else {
				printf("A pirate arrived at %d hours\n", arrival_time);
				addToQueue(&pirateQueue, thread_id, arrival_time);
				
		}	
		
			printQueues();
	pthread_mutex_unlock(&queueLock);

//	pthread_mutex_lock(&fittedLock);
	
		while(atMaxOccupancy(roomLock));
		
		struct ArrivalNode *next = NULL;
		
		enum Team team = NA;
		
			printQueues();
			
			while(next == NULL) {
					
				pthread_mutex_lock(&queueLock);
					if(room_lock_will_accept(roomLock) == NINJAS && countQueueSize(&ninjaQueue) > 0) {
						next = popHead(&ninjaQueue);
						team = NINJAS;
					} else if(room_lock_will_accept(roomLock) == PIRATES && countQueueSize(&pirateQueue) > 0) {
						next = popHead(&pirateQueue);
						team = PIRATES;
					}	
				pthread_mutex_unlock(&queueLock);
			}
			
		room_lock_and_wait(roomLock, next, team);


		printf("\t\t\t\t\t=====] Earnings: $%d\n", earnings); 

	return NULL;
}

void room_lock_and_wait(room_lock_t *lock, struct ArrivalNode *node, enum Team team) {
	if(room_lock_will_accept(lock) == team || room_lock_will_accept(lock) == NA) {
		room_lock_acquire_lock(lock, team);
		
		//pthread_mutex_unlock(&fittedLock);
		
		unsigned int changeTime = rand() % 5;
		earnings += changeTime;
		
		printf("Thread #%d on team #%d is going to change for %d seconds\n", node->thread_id, team, changeTime);
		sleep(changeTime);
		printf("A Team #%d has exited the room\n", team);
		
		room_lock_release_lock(lock);
	}
	
}

int main(int argc, char** argv) {
	
		srand(time(0));
		money = 0;
		
		numTeams = atoi(argv[1]);
		numPirates = atoi(argv[2]);
		numNinjas = atoi(argv[3]);
		
		start = 0;
		
		costumeLock = malloc(sizeof(pthread_mutex_t) * numTeams);
		
		pirates = malloc(sizeof(pthread_t) * numPirates);
		ninjas = malloc(sizeof(pthread_t) * numPirates);
		
		makeThreads();
		//create the requiste threads
			
		pthread_exit(NULL);
		
		printf("Gold coins earned %d\n", money);
		
		pthread_mutex_destroy(&queueLock);
	
}
void makeThreads() {
	
	roomLock = malloc(sizeof(room_lock_t));
	room_lock_init(roomLock, numTeams);
	
	if(pthread_mutex_init(&queueLock, NULL) != 0) {
			printf("queueLock failed\n");
	}
	
	pthread_mutex_init(&fittedLock, NULL);
	
	printf("Making threads..\n");
	
	for(int x = 0; x < numPirates; x++) {
		int *arg = malloc(sizeof(int));
		*arg = x;
		pthread_create(&pirates[x], NULL, arrive, (void *) arg);
	}

	for(int x = 0; x < numNinjas; x++) {
		int *arg = malloc(sizeof(int));
		*arg = x + numPirates;
		pthread_create(&ninjas[x], NULL, arrive, (void *) arg);
	}
	
	start = 1;
}
unsigned int isNinja(int thread_id) {
	return numPirates <= thread_id;
}
void addToQueue(struct Queue *queue, unsigned int thread_id, unsigned int arrivalTime) {
	
	if(queue->front == NULL) {
		struct ArrivalNode* temp = (struct ArrivalNode*) malloc(sizeof(struct ArrivalNode)); 
		temp->thread_id = thread_id; 
		temp->arrivalTime = arrivalTime;
		queue->front = temp;
		queue->rear = temp;
		
		return;
	}
	//adds to the proper queue, make sure it is ordered by arrival time.	
	struct ArrivalNode* head = queue->front;
	
	struct ArrivalNode* temp = (struct ArrivalNode*)malloc(sizeof(struct ArrivalNode)); 
    temp->thread_id = thread_id; 
    temp->arrivalTime = arrivalTime;
	//if this arrives earlier than head, make this the first element
	
	if (head->arrivalTime > arrivalTime){
		temp->next = head;
		queue->front = temp;
		return;
	}
	while (head->next != NULL){
		//if this arrives eariler than next, insert
		if ((head->next)->arrivalTime > arrivalTime)
			break;
		head = head->next;
	}
	temp->next = head->next;
	if (temp->next == NULL)
		queue->rear = temp;
	head->next = temp;
}
struct ArrivalNode* popHead(struct Queue *queue) {
		//pops off the head of the queue

	if(queue->front == NULL) {
		return NULL;
	}

	struct ArrivalNode *ret = queue->front;
	queue->front = (queue->front)->next;
	return ret;
}
unsigned int countQueueSize(struct Queue *queue) {
		unsigned int count = 0;
		
		struct ArrivalNode *current = queue->front;
		
		while(current != NULL) {
				count++;
				current = current->next;
		}
		
		return count;
	
}

void printQueues() {
		
		printf("\n\nRoom occupancy (max %d): %d Allowed team: %d Next team: %d\n", roomLock->max, roomLock->occupants, roomLock->currentTeam, roomLock->nextTeam);
		
		struct ArrivalNode *p = ninjaQueue.front;
		
		if(p != NULL) {
			printf("\nNinja Queue(%d): ", countQueueSize(&ninjaQueue));
			for(int x = 0; x < countQueueSize(&ninjaQueue); x++) {
					printf("#%d <- ", p->thread_id);
					p = p->next;
			}
		}
			
		p = pirateQueue.front;
		
		if(p != NULL) {
			printf("\nPirate Queue (%d): ", countQueueSize(&pirateQueue));
			for(int x = 0; x < countQueueSize(&pirateQueue); x++) {
					printf("#%d <- ", p->thread_id);
					p = p->next;
			}
		}
		
		printf("\n");
}
