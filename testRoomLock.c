
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include <pthread.h>

#include <time.h>

#include <semaphore.h>

#include "room_lock.c"


room_lock_t roomLock;


pthread_t ppl[10];
void *arrive(void *argp) {

	room_lock_acquire_lock(&roomLock);
	sleep(4);
	room_lock_release_lock(&roomLock);

}


int main(int argc, char **argv) {

	room_lock_init(&roomLock, 4); //for teams are in the department

	for(int x = 0; x < 10; x++) {
		pthread_create(ppl, NULL, arrive, (void *) &ppl);
	}
	
	pthread_exit(NULL);
}
