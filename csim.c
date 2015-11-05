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
	int option,set,block,lines,size; //input options
	char *fileName; //name of file to open
	FILE *myFile;
	char  ch;
	int t;      	//Tag bits
	int hit=0; 	//To determine if a Hit has occured
	unsigned long long int address; //Address from file
	unsigned long long int temp, setNum,addrTag;
	int unused =-1;

        typedef struct {
                int validBit;
                unsigned long long int tag;
                unsigned long long int LRU;
        }lineStr;

        typedef struct {
        	lineStr *line;          //pointer to array of lines
        }setStr;

        typedef struct {
                setStr *sets;	//point to array of sets
        } cacheStr;

	// Parse Options using getopts
	while(((option = getopt(argc, argv,"s:E:b:t:")) != -1)) {
		switch(option){
			case 's': 
				set=pow(2,atoi(optarg));
				break;
			case 'E': 
				lines= atoi(optarg);
	                 	break;
			case 'b': 
				block=pow(2,atoi(optarg));
	                      	break;
			case 't': 
				fileName=optarg;
	    			break;
			default:
				break;
		}
	}
	// Check all required info is there
	if(set == 0 || lines==0 || block == 0 || fileName ==NULL) {
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
	while(fscanf(myFile, " %c %llx, %d", &ch, &address, &size) ==3) {
		int evict =0;
		int oldest = 99;
		if ( ch !='I') {
			t = 64 -( set + block);
			addrTag = address >> (set + block);
			temp = address << t;
			setNum = temp >> (t + block);
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
				else if (unused == -1) {	//set to first unused space
					unused =i;
				}
			}
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
		
		if (ch =='M')
			hit_count++;
		unused =-1;
		hit = 0;
		}	
	
	}	
	// Free Memory
	free(cache.sets);
	fclose(myFile);
	
    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

