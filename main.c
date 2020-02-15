
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>



// Supplied parameters
unsigned int numTeams; 
unsigned int numPirates;
unsigned int numNinjas;

//use a linked list to maintain the detail of each entry
struct Visit {
	unsigned int entityID;

	unsigned int entryTime;
	unsigned int exitTime;
	
	//... mas aqui luego
};


int main(int argc, char** argv) {
		numTeams = atoi(argv[1]);
		numPirates = atoi(argv[2]);
		numNinjas = atoi(argv[3]);
		
		
		//create the requiste threads
		
		
}

/*
 * Notes:
 * 
 * 
 * 
 * 
 * /

