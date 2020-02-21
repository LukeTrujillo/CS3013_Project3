
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include <pthread.h>

#include <time.h>

#include <semaphore.h>

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



//use a linked list to maintain the detail of each entry
struct Visit {
	unsigned int entityID;

	unsigned int entryTime;
	unsigned int exitTime;
	
	//... mas aqui luego
};



struct ArrivalNode {
		struct ArrivalNode *next;
		unsigned int arrivalTime;
		unsigned int thread_id;
};

struct Queue { 
    struct ArrivalNode *front, *rear; 
}; 


unsigned int isNinja(int*);
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

unsigned int ninjaTurn;

void *arrive(void *vargp) {
	int arrival_time = rand() % 24;
	
	while(!start);
	
	sleep(arrival_time);
	
	int *thread_id = (int *) vargp;
	
	pthread_mutex_lock(&queueLock);
	
	if(isNinja(thread_id)) {
			printf("A ninja arrived at %d hours\n", arrival_time);
			
			addToQueue(&ninjaQueue, *thread_id, arrival_time);

			
	} else {
			printf("A pirate arrived at %d hours\n", arrival_time);
			addToQueue(&pirateQueue, *thread_id, arrival_time);

	}	
	pthread_mutex_unlock(&queueLock);
	
	
	if(isNinja(thread_id))
		sem_wait(&ninjaSem);
	if(!isNinja(thread_id)) {
		sem_wait(&pirateSem);
	}
		
	pthread_mutex_lock(&fittedLock);
	
	while(!openRoom());
	
	if(isNinja(thread_id)) getFitted(popHead(&ninjaQueue)->thread_id);
	else getFitted(popHead(&pirateQueue)->thread_id);
	
	decide();

	
	return NULL;
}

//This should choose whether or not you switch teams
void decide() {
	
	unsigned int ninjaSize = countQueueSize(&ninjaQueue);
	unsigned int pirateSize = countQueueSize(&pirateQueue);
	
	
	printf("\nninjaQ: %d pirateQ: %d ", ninjaSize, pirateSize);
	
	
	if(ninjaSize >= numTeams && ninjaSize != 0) {
			if(pirateSize >= numTeams) {
				
				
				unsigned urgencyNinjaSize = (ninjaQueue.front)->arrivalTime;
				unsigned urgencyPirateSize = (pirateQueue.front)->arrivalTime;
				
				
				if(urgencyNinjaSize < urgencyPirateSize) {
					printf("choose ninjas\n");
					sem_post(&ninjaSem);
				} else {
					printf("choose pirates\n\n");
					sem_post(&pirateSem);
				}
				
				
			} else {
				printf("choose ninjas\n");
				sem_post(&ninjaSem);
			}
	} else if(pirateSize >= numTeams) {
		printf("choose pirates\n\n");
		sem_post(&pirateSem);
	}

}

unsigned int openRoom() {
	
	for(int x = 0; x < numTeams; x++) {
		if(pthread_mutex_trylock(&costumeLock[x]) != 0) continue;
		
		pthread_mutex_unlock(&costumeLock[x]);
		return 1;
	}
	return 0;
}

void getFitted(int thread_id) {

	pthread_mutex_unlock(&fittedLock);

	for(int x = 0; x < numTeams; x++) {
		if(pthread_mutex_trylock(&costumeLock[x]) != 0) continue;
	
		
		int time = rand() % 12;
		printf("Thread ID %d is being fitted by Team #%d for %d seconds.\n", thread_id, x, time);
		//choose the amount of time to be in the room
	
	
		sleep(time);
		
		money += time;
	
		printf("Opening up Team #%d\n", x);
		//now create entry and charge the client
			
		pthread_mutex_unlock(&costumeLock[x]);
		break;
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
		
		for(int x = 0; x < numTeams; x++) {
			pthread_mutex_destroy(&costumeLock[x]);
		}
	
}
void makeThreads() {
	if(pthread_mutex_init(&queueLock, NULL) != 0) {
			printf("queueLock failed\n");
	}
	
	for(int x = 0; x < numTeams; x++) {
		if(pthread_mutex_init(&costumeLock[x], NULL) != 0) {
			printf("num lock done failed\n");
		}
	}
	pthread_mutex_init(&accessLock, NULL);
	pthread_mutex_init(&fittedLock, NULL);
	pthread_mutex_init(&withhold_pirates, NULL);
	pthread_mutex_init(&teamLock, NULL);
	
	sem_init(&ninjaSem, 0, numNinjas);
	sem_init(&pirateSem, 0, numPirates);
	
	
	printf("Making threads..\n");
	
	for(int x = 0; x < numPirates; x++) {
		pthread_create(&pirates[x], NULL, arrive, (void *) &pirates[x]);
	}

	for(int x = 0; x < numNinjas; x++) {
		pthread_create(&ninjas[x], NULL, arrive, (void *) &ninjas[x]);
	}
	
	gameClock = time(NULL); //start the timer for the game
	start = 1;\
	decide();
	
}
unsigned int isNinja(int *thread_id) {
	
	for(int x = 0; x < numNinjas; x++) {
			if(ninjas[x] == *thread_id) return 1;
	}
	return 0;
}
void addToQueue(struct Queue *queue, unsigned int thread_id, unsigned int arrivalTime) {
	
	
	if(queue->front == NULL) {
		struct ArrivalNode* temp = (struct ArrivalNode*)malloc(sizeof(struct ArrivalNode)); 
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
