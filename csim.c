/* 
 * csim.c - A cache simulator that can replay traces from Valgrind
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is LRU.
 *
 * Implementation and assumptions:
 *  1. Each load/store can cause at most one cache miss. (I examined the trace,
 *  the largest request I saw was for 8 bytes).
 *  2. Instruction loads (I) are ignored, since we are interested in evaluating
 *  trans.c in terms of its data cache performance.
 *  3. data modify (M) is treated as a load followed by a store to the same
 *  address. Hence, an M operation can result in two cache hits, or a miss and a
 *  hit plus an possible eviction.
 *
 * The function printSummary() is given to print output.
 * Please use this function to print the number of hits, misses and evictions.
 * This is crucial for the driver to evaluate your work. 
 */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 0;

/* Main Program */
int main(int argc, char *argv[])
{
	int option,s,b,lines,size; //input options
	int set, block; //For bit shifting
	char *fileName; //name of file to open
	FILE *myFile; 
	char  ch;       //Storage of action element of input
	int t;      	//Number of Tag bits to shift
	int hit=0; 	//To determine if a Hit has occured bool var not available
	int evict, oldest; //To determine which address to evict based on LRU
	unsigned long long int address; //Address from file
	unsigned long long int temp, setNum,addrTag; //needed for address calculation
	int unused =-1;  //Used for index in cache to find unused line

	//Structures for cache elements
        typedef struct {
                int validBit;  //0 if no address present, 1 if address present
                unsigned long long int tag; //calculated tag for the stored address
                unsigned long long int LRU; //value for replacement policy
        } lineStr;

        typedef struct {
        	lineStr *line;          //pointer to array of lines
        } setStr;

        typedef struct {
                setStr *sets;	//point to array of sets
        } cacheStr;

	// Parse Options using getopts
	while(((option = getopt(argc, argv,"s:E:b:t:")) != -1)) {
		switch(option){
			case 's': 
				s=atoi(optarg);
				set=pow(2,s);
				break;
			case 'E': 
				lines= atoi(optarg);
	                 	break;
			case 'b': 
				b = atoi(optarg);
				block=pow(2,b);
	                      	break;
			case 't': 
				fileName=optarg;
	    			break;
			default:
				printf("Invalid option input. Exiting program.");
				exit(-1);
				break;
		}
	}
	// Check all required info is there
	if(s == 0 || lines==0 || b == 0 || fileName ==NULL) {
		printf("Missing input neccessary for computation.");
	}
	myFile =fopen(fileName,"r");

	// Initialize Cache using malloc
		
   	cacheStr cache;
   	cache.sets = malloc(sizeof(setStr) * set);
 	int i;
  	for (i = 0; i < set; i++ ) {
      		cache.sets[i].line = malloc( sizeof (lineStr) * block);
   	}	
	//Read the input and store in for use
	while(fscanf(myFile, " %c %llx, %d", &ch, &address, &size) !=EOF) {
		//reset variables
                unused =-1;
                hit = 0;	
		evict =0;
		oldest = 9999;
		
// M will have a hit for the S so we automatically add a hit and go through the testing
		if ( ch == 'M')
			hit_count++;
		if ( ch !='I') {
			t = 64 -( s + b);
			addrTag = address >> (s + b);
			temp = address << t;
			setNum = temp >> (t + b);
			setStr cacheSet=cache.sets[setNum];
			for(i=0; i<lines;i++) {
				if (cacheSet.line[i].validBit ==1) {
					if (cacheSet.line[i].tag == addrTag) {
						cacheSet.line[i].LRU =lru_counter;
						hit =1;
						hit_count++;
						cacheSet.line[i].LRU=lru_counter;
						lru_counter++;
					}
					else if (cacheSet.line[i].LRU < oldest) {
						evict=i;
						oldest= cacheSet.line[i].LRU;	
					}
				}
				 //set to first unused space
				else if (unused == -1) {	
					unused =i;
				}
			}
			//check if there was a miss if so do further testing
			if (hit !=1) {
				miss_count++;
				if(unused > -1) {
					cacheSet.line[unused].validBit =1;
					cacheSet.line[unused].tag=addrTag;
					cacheSet.line[unused].LRU=lru_counter;
				}
				//Evict if no availabe spots to fill
				else if( unused < 0) {
					cacheSet.line[evict].tag=addrTag;
					cacheSet.line[evict].LRU=lru_counter;
					eviction_count++;
				}
				lru_counter++;
			}
		}	
	
	}	
	// Free Memory
	free(cache.sets);
	fclose(myFile);
	
    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

