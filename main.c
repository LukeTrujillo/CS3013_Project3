
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
unsigned int pirateCostumeTime;
unsigned int ninjaCostumeTime;
unsigned int pirateArrivalTime;
unsigned int ninjaArrivalTime;

unsigned int oP;
unsigned int oN;

unsigned int money;
unsigned int payVar = 0; //variable for testAndSet while paying


// Holds the TID (Thread ID)
pthread_t *pirates;
pthread_t *ninjas;

// Functions
void makeThreads();

void printQueues();


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
void pay(int amount);

unsigned long gameClock;
unsigned int earnings = 0;
unsigned int ninjaTurn;

void room_lock_and_wait(room_lock_t*, struct ArrivalNode*, enum Team);

room_lock_t *roomLock; 


unsigned int addOn = 0;


void think();

int getArrivalTime(int thread_id){
	int arrivalTime = 0;
	if (isNinja(thread_id)){
		arrivalTime = ninjaArrivalTime;	
	}	
	else {
		arrivalTime = pirateArrivalTime;
	}
	return (rand() % (2 * arrivalTime - 2)) + 1; 
}

void *arrive(void *vargp) {

	int thread_id = *((int *) vargp);
	int arrival_time = getArrivalTime(thread_id);

	while(!start);
	
	sleep(arrival_time);

	
	if(gameClock < arrival_time) {
			gameClock = arrival_time;
	}
	
	
	
	pthread_mutex_lock(&queueLock);
	
	
		if(isNinja(thread_id)) {
				//printf("Ninja %d arrived the the store\n", thread_id);
				
				addToQueue(&ninjaQueue, thread_id, arrival_time);

				
		} else {
				//printf("Pirate %d arrived the the store\n", thread_id);
				addToQueue(&pirateQueue, thread_id, arrival_time);
				
		}	
		
		//printQueues(); 

			
			
	pthread_mutex_unlock(&queueLock);

	pthread_mutex_lock(&fittedLock);
	 
		struct ArrivalNode *next = NULL;
		
		enum Team team;
			
			
			
			while(atMaxOccupancy(roomLock) && countQueueSize(&ninjaQueue) > 0 && countQueueSize(&pirateQueue) > 0);
					
			while(next == NULL) {
				
				pthread_mutex_lock(&queueLock);
					if(room_lock_will_accept(roomLock, NINJAS) && countQueueSize(&ninjaQueue) > 0) {
					
						next = popHead(&ninjaQueue);
					team = NINJAS;
						
					} else if(room_lock_will_accept(roomLock,PIRATES) && countQueueSize(&pirateQueue) > 0) {
						next = popHead(&pirateQueue);
						team = PIRATES;
					}	
				pthread_mutex_unlock(&queueLock);
				
			}
			
			pthread_mutex_unlock(&fittedLock);
			
			room_lock_and_wait(roomLock, next, team);
			


	return NULL;
}
int ret = 100;

int getCostumeTime(int thread_id){
	int costumeTime = 0;
	if (isNinja(thread_id)){
		costumeTime = ninjaCostumeTime;	
	}	
	else {
		costumeTime = pirateCostumeTime;
	}
	return (rand() % (2 * costumeTime - 2)) + 1;
}

void room_lock_and_wait(room_lock_t *lock, struct ArrivalNode *node, enum Team team) {
	while(!room_lock_will_accept(lock, team));
		

		struct Visit *visit = (struct Visit*) malloc(sizeof(struct Visit));

		unsigned int changeTime = getCostumeTime(node->thread_id);
		if (node->arrivalTime < 30)
			pay(changeTime);

		
		visit->arrival = node;
		visit->timeSpent = changeTime;
		visit->entity = node->thread_id;
		
		if(isNinja(node->thread_id)) { visit->entityTeam = NINJAS;}
		else visit->entityTeam = PIRATES;
		
		visit->servicedBy = NULL;
		visit->next = NULL;
		
		
		room_lock_acquire_lock(lock, visit);
		
		earnings += changeTime;
	
		sleep(changeTime); 
		
		if(gameClock < changeTime + node->arrivalTime) { gameClock = changeTime + node->arrivalTime; }
		room_lock_release_lock(lock, visit);
		 
		think();
		
		unsigned recycle = rand() % 4;
		
		if(recycle == 0) { 
			
			addOn = 1;
			pthread_t *newThread = (pthread_t *) malloc(sizeof(pthread_t));
			int *num = (int *) malloc(sizeof(int));
			
			*num = node->thread_id;
			
			if(isNinja(node->thread_id)) {
					numNinjas++;
					ninjas = (pthread_t *) realloc(ninjas, sizeof(pthread_t) * numNinjas);
					
					pthread_create(&ninjas[numNinjas - 2], NULL, arrive, (void *) num);
			} else {
				numPirates++;
				pirates = (pthread_t *) realloc(pirates, sizeof(pthread_t) * numPirates);
				
				pthread_create(&pirates[numPirates- 2], NULL, arrive, (void *) num);
			}
		
		}
		
		
		pthread_exit(NULL);
}

int testAndSet(int * target, int value) {
	int oldvalue = *target;
	*target = value;
	return oldvalue;
}

void pay(int amount){
	while (testAndSet(&payVar, 1) == 1){
		;
	}
	earnings += amount;
	testAndSet(&payVar, 0);
	return;
}

int main(int argc, char** argv) {
	
		srand(time(NULL));
		money = 0;
		
		numTeams = atoi(argv[1]);
		numPirates = atoi(argv[2]);
		numNinjas = atoi(argv[3]);
		
		oP = numPirates;
		oN = numNinjas;
		

		pirateCostumeTime = atoi(argv[4]);
		ninjaCostumeTime = atoi(argv[5]);
		pirateArrivalTime = atoi(argv[6]);
		ninjaArrivalTime = atoi(argv[7]);

		start = 0;
		
		costumeLock = malloc(sizeof(pthread_mutex_t) * numTeams);
		
		pirates = malloc(sizeof(pthread_t) * numPirates);
		ninjas = malloc(sizeof(pthread_t) * numPirates);
		
		makeThreads();
			
		//WAIT UNTIL ALL THREADS ARE DONE
		
		for(int x = 0; x < numPirates; x++) {
				pthread_join(pirates[x], NULL);
		}
		for(int x = 0; x < numNinjas; x++) {
				pthread_join(ninjas[x], NULL);
		}
		
		
		printf("\n\n -- End of Simulation --\n\n");


	unsigned int earn = 0;
	
	unsigned int totalNumVisits = 0;
		for(int x = 0; x < oN + oP; x++) {
			if(x < oN) printf("Ninja %d:\n", x);
			else printf("Pirates %d:\n", x);
			
			struct Visit *current = roomLock->head;
			
			
			int numVisits = 1;
			int time = 0;
			while(current->next != NULL) { 
				if(current->entity == x) {
						printf("\tVisit %d: Waited for %d minutes and was in the shop for %d minutes\n", numVisits, current->arrival->arrivalTime, current->timeSpent);
						
						
						if(current->arrival->arrivalTime < 30) { time += current->timeSpent; }
						
						totalNumVisits++;
				
						
						numVisits++;
				}
				
				current = current->next;
			}
			earn += time;
			
			
			printf("\tTotal # of visits: %d. Cost for person: %d\n", numVisits, time);
			
			
			if(x + 1 == oN){ printf("The total for Ninjas was %d\n", earn); earn = 0; }
		}
		
		printf("The total for Pirates was %d\n\n", earn);
		
		printf("Expenses for employing the team is %d\n", numTeams * 5);
		
		for(int x = 0; x < numTeams; x++) {
			printf("Team %d was busy for %d minutes \n", x, roomLock->costumeTeams[x].timeSpentBusy);
		}
		
		printf("Gross revenue is %d gold pieces\n", earnings);
		
		printf("Gold per visit: %d\n", earnings / totalNumVisits);
		printf("Total revenue: %d", earnings - (numTeams * 5));
		
		
		
		pthread_mutex_destroy(&queueLock);
		pthread_mutex_destroy(&fittedLock);
	
}
void makeThreads() {


	roomLock = malloc(sizeof(room_lock_t));
	room_lock_init(roomLock, numTeams);
	
	if(pthread_mutex_init(&queueLock, NULL) != 0) {
			printf("queueLock failed\n");
	}
	
	pthread_mutex_init(&fittedLock, NULL);
	
	
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

void think() {

	
		unsigned int threshold = 20;
		
		unsigned int nQ = countQueueSize(&ninjaQueue);
		unsigned int pQ = countQueueSize(&pirateQueue);
		
		if(nQ == 0 && pQ != 0) room_lock_set_team(roomLock, PIRATES);
		
		if(pQ == 0 && nQ != 0) room_lock_set_team(roomLock, NINJAS);
		
		
		if(ninjaQueue.front != NULL && nQ > 0 && gameClock - ninjaQueue.front->arrivalTime > threshold) {
			room_lock_set_team(roomLock, NINJAS);
		} else 	if(pirateQueue.front != NULL && pQ > 0 && gameClock - pirateQueue.front->arrivalTime > threshold) {
			room_lock_set_team(roomLock, PIRATES);
		}
		
		if(pQ > 0 && ((nQ < 4 && pQ > 4) || (pQ - nQ > 4))) {
			room_lock_set_team(roomLock, PIRATES);
		} else if(nQ > 0 && ((nQ > 4 && pQ < 4) || (pQ - nQ > 4))) {
			room_lock_set_team(roomLock, NINJAS);
		} 
			 
			
	
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
		
		printf("\n\nRoom occupancy (max %d): %d Allowed team: %d Swap Flag: %d\n", roomLock->max, getOccupancy(roomLock), roomLock->currentTeam, roomLock->swapTrigger);
		
		for(int x = 0; x < numTeams; x++) {
				printf("%d ", roomLock->costumeTeams[x].serving);
			
		}
		printf("\n");
		
		
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
