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
unsigned long long int lru_counter = 1;

/* Main Program */
int main(int argc, char *argv[])
{
	int option,set,block,lines;
	char *fileName;
	FILE *myFile;


	// Parse Options using getopts
	while(((option = getopt(argc, argv,"s:E:b:t:")) != -1)) {
		switch(option){
			case 's': 
				set=pow(2,atoi(optarg));
				printf("S %d ",set); // for testing
				break;
			case 'E': 
				lines= atoi(optarg);
				printf("E %d ",lines); // for testing
                        	break;
			case 'b': 
				block=pow(2,atoi(optarg));
				printf("B %d ",block); // for testing
                        	break;
			case 't': 
				fileName=optarg;
				myFile =fopen(fileName,"rw");
				printf("File open %s",fileName); // for testing
                        	fclose(myFile);
				break;
			default:
				break;
		}



	}
	// Check all required info is there
	
	// Initialize Cache using malloc
	
	// Free Memory

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
