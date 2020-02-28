#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

enum Team {NINJAS, PIRATES, NA};

struct ArrivalNode {
		struct ArrivalNode *next;
		unsigned int arrivalTime;
		unsigned int thread_id;
};


typedef struct _room_lock_t {
	int max;
	int occupants;
	sem_t room_lock;
	sem_t lock;
	
	enum Team currentTeam;
	enum Team nextTeam;
	
} room_lock_t;

void room_lock_init(room_lock_t *rl, unsigned int teams) {
	rl->max = teams;
	rl->occupants = 0;
	sem_init(&rl->room_lock, 0, teams); // allow the number of teams to enter the room
	sem_init(&rl->lock, 0, 1);


	srand(time(NULL));
	
	unsigned int choice = rand() % 2;
	
	if(choice) {
		 rl->currentTeam = PIRATES;
		 rl->nextTeam = NINJAS;
	}
	else {
		 rl->currentTeam = NINJAS;
		 rl->nextTeam = PIRATES;
	}

}

void room_lock_change_team(room_lock_t *lock, enum Team attempt) {
	sem_wait(&lock->lock);
	
		if(lock->nextTeam != NA) {
			printf("Team #%d is already queued so the call to room_lock_change_team is ignored\n", lock->nextTeam);
		} else if(attempt == lock->nextTeam) {
			printf("Team #%d is already set to be the next team this room_lock_change_team is ignored\n", lock->nextTeam);
		} else if(lock->nextTeam == NA) {
			lock->nextTeam = attempt;
			printf("Next team has been set to Team#%d\n", attempt);
		}
	
	
	sem_post(&lock->lock);
}

enum Team room_lock_will_accept(room_lock_t *lock) {
	return lock->currentTeam; //will need to change later for dynamic switching
}

void room_lock_acquire_lock(room_lock_t *lock, enum Team attempt) {
	sem_wait(&lock->lock); //only one thread at a time can enter here
	
	if(lock->currentTeam == NA) {
		lock->currentTeam = attempt;
	}
	
	if(attempt != lock->currentTeam) {
		sem_post(&lock->lock);
		return;
	} 
	
	lock->occupants++;

	if(lock->occupants > lock->max) {
		printf("Max capacity for room achieved...all threads calling room_lock_acquire_room_lock() will be waiting\n");
		sem_wait(&lock->room_lock);
	} else {
			printf("\tTeam #%d has entered the room.\n", attempt);
	}
	
	sem_post(&lock->lock);
}
void room_lock_release_lock(room_lock_t *lock) {
	sem_wait(&lock->lock); //only one thread can enter here
	lock->occupants--;

	if(lock->occupants == 0) {
		
		if(lock->nextTeam != NA) {
			lock->currentTeam = lock->nextTeam;
			lock->nextTeam = !lock->currentTeam;
			printf("The team for the room has been changed to team #%d\n", lock->currentTeam);
		}
		
		sem_post(&lock->room_lock);
	}
	
	printf("\tOne person left the room\n");

	sem_post(&lock->lock);
}

unsigned int atMaxOccupancy(room_lock_t *lock) {
	unsigned int max;

	sem_wait(&lock->lock); //only one thread at a time
	max = lock->max;
	sem_post(&lock->lock);

	return max == lock->occupants;
}
