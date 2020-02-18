
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include <pthread.h>



// Supplied parameters
unsigned int numTeams; 
unsigned int numPirates;
unsigned int numNinjas;

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


pthread_mutex_t queueLock;
pthread_mutex_t *costumeLock;

pthread_cond_t ninjaOnlyCondition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t accessLock;



void getFitted();

struct Queue pirateQueue;
struct Queue ninjaQueue;




void *arrive(void *vargp) {
	printf("Arrive() called...\n");
	
	int arrival_time = rand() % 24;
	
	while(!start);
	
	sleep(arrival_time);
	
	int *thread_id = (int *) vargp;
	
	pthread_mutex_lock(&queueLock);

	
	
		if(isNinja(thread_id)) {
				printf("A ninja arrived at %d hours\n", arrival_time);
	
				
		} else {
				printf("A pirate arrived at %d hours\n", arrival_time);

		}
		
			
		pthread_mutex_lock(&accessLock);
		
			if(isNinja(thread_id)) {
				printf("A ninja is being served at %d hours\n", arrival_time);
	
					
			} else {
					printf("A pirate served at %d hours\n", arrival_time);
			}
			
			getFitted();
		
		
		
		pthread_mutex_unlock(&accessLock);
		
	
	pthread_mutex_unlock(&queueLock);
	
	
	return NULL;
}

void getFitted() {



	for(int x = 0; x < numTeams; x++) {
		if(pthread_mutex_trylock(&costumeLock[x]) != 0) continue;
		
		
		printf("Fitted by team %d\n", 0) ;
		//choose the amount of time to be in the room
		
		unsigned int time = 2;
	
		sleep(time);
		
		
		printf("leaving\n");
		//now create entry and charge the client
			
		pthread_mutex_unlock(&costumeLock[x]);
		break;
	}
}


int main(int argc, char** argv) {
	
		srand(time(0));
		
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
	
	
	printf("Making threads..\n");
	
	for(int x = 0; x < numPirates; x++) {
		pthread_create(&pirates[x], NULL, arrive, (void *) &pirates[x]);
	}

	for(int x = 0; x < numNinjas; x++) {
		pthread_create(&ninjas[x], NULL, arrive, (void *) &ninjas[x]);
	}
	
	start = 1;
	
}

unsigned int isNinja(int *thread_id) {
	
	for(int x = 0; x < numNinjas; x++) {
			if(ninjas[x] == *thread_id) return 1;
	}
	return 0;
}

void addToQueue(struct Queue *queue, unsigned int thread_id, unsigned int arrivalTime) {
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
	while (head != NULL){
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

