
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
}

struct Queue { 
    struct ArrivalNode *front, *rear; 
}; 


unsigned int isNinja(int*);
unsigned int start;


struct ArrivalNode *pirateHead;
struct ArrivalNode *ninjaHead;
struct Queue pirateQueue;
struct Queue ninjaQueue;



void *arrive(void *vargp) {
	printf("Arrive() called...\n");
	
	while(!start);
	
	srand(time(0));
	
	int arrival_time = rand() % 24;
	
	sleep(arrival_time);
	
	
	if(isNinja(((int *) vargp))) {
			printf("A ninja ");
	} else {
			printf("A pirate ");
	}
	
	printf("arrived at %d hours\n", arrival_time);
	
	//queue them here
	
	
	
	return NULL;
}


int main(int argc, char** argv) {
		numTeams = atoi(argv[1]);
		numPirates = atoi(argv[2]);
		numNinjas = atoi(argv[3]);
		
		start = 0;
		
		
		pirates = malloc(sizeof(pthread_t) * numPirates);
		ninjas = malloc(sizeof(pthread_t) * numPirates);
		
		makeThreads();
		//create the requiste threads
		
		pthread_exit(NULL);
}

void makeThreads() {
	
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
	struct ArrivalNode* head = queue->front
	struct ArrivalNode* temp = (struct ArrivalNode*)malloc(sizeof(struct ArrivalNode)); 
    temp->thread_id = thread_id; 
    temp->arrivalTime = arrivalTime;
	//if this arrives earlier than head, make this the first element
	if (head->arrivalTime > arrivalTime){
		temp->next = head;
		queue->head = temp;
		return
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
	struct ArrivalNode ret = queue->first;
	queue->first = (queue->first)->next;
	return ret;
}
