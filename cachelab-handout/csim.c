#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct cacheLine {
    int valid_bit;
    int tag;
    int LRU_counter;
} cache_line;

void cache_initialize(int numSets, int E, struct cacheLine cache[numSets][E]) {
    for (int i = 0; i < numSets; i++) {
        for (int j = 0; j < E; j++) {
            cache[i][j].valid_bit = 0;
            cache[i][j].LRU_counter = 0;
        }
    }
}

void initialize_full(int numSets, int full[]) {
    for (int i = 0; i < numSets; i++) {
        full[i] = 0;
    }
}

int exists_in_cache(int set, int tag, int numSets, int E, struct cacheLine cache[numSets][E]) {
    for (int j = 0; j < E; j++) {
        if (cache[set][j].valid_bit == 1 && cache[set][j].tag == tag) {
            cache[set][j].LRU_counter = 0;
            return 1;
        }
    }
    return 0;
}

int insert(int set, int tag, int numSets, int E, struct cacheLine cache[numSets][E]) {
    for (int j = 0; j < E; j++) {
        if (cache[set][j].valid_bit == 0) {
            cache[set][j].valid_bit = 1;
            cache[set][j].tag = tag;
            cache[set][j].LRU_counter = 0;
            return 1;
        }
    }
    return 0;
}

void evict_and_replace(int set, int tag, int numSets, int E, struct cacheLine cache[numSets][E]) {
    int max = 0;
    struct cacheLine* ptr;
    for (int j = 0; j < E; j++) {
        if (cache[set][j].valid_bit == 1) {
            if (cache[set][j].LRU_counter > max) {
                max = cache[set][j].LRU_counter;
                ptr = &cache[set][j];
            }
        }
    }
    ptr->tag = tag;
    ptr->LRU_counter = 0;
}

void update_LRU(int numSets, int E, struct cacheLine cache[numSets][E]) {
    for (int i = 0; i < numSets; i++) {
        for (int j = 0; j < E; j++) {
            cache[i][j].LRU_counter++;
        }
    }
}

int main(int argc, char** argv)
{
    int v = 0;
    int opt, s, E, b;
    char* trace;
    while(-1 != (opt = getopt(argc, argv, "vs:E:b:t:"))){
        switch(opt) {
            case 'v':
                v = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace = optarg;
                break;
            default:
                printf("wrong argument\n");
                break;
            }
    }
    int numSets = (1 << s);
    //int numBlocks = (1 << b);
    struct cacheLine cache[numSets][E];
    cache_initialize(numSets, E, cache);

    int hit = 0;
    int miss = 0;
    int evict = 0;
    int full[numSets];
    initialize_full(numSets, full);

    FILE* pFile;	//pointer	to	FILE	object	
    pFile = fopen(trace,"r");	//open	file	for	reading	
    char identifier;	
    unsigned address;	
    int	size;
    unsigned set_mask = ((1 << s) - 1) << b;
    //	Reading	lines	like	"	M	20,1"	or	"L	19,3"	
    while(fscanf(pFile," %c %x,%d",	&identifier, &address, &size) > 0)	
    {	
        if (identifier == 'I') continue;
        /* 先来一遍cache search */
        int set = (address & set_mask) >> b;
        int tag = address >> (b + s);
        /* if current address is in cache (hit)*/
        if (exists_in_cache(set, tag, numSets, E, cache)) {
            if (v) {
                printf("%c %x,%d %s", identifier, address, size, "hit ");
                printf("\n");
            }
            hit++;
        }
        /* if current address is not in cache (miss)*/
        else {
            if (v) {
                printf("%c %x,%d %s", identifier, address, size, "miss ");
            }
            miss++;
            if (!full[set]) {
                if (!insert(set, tag, numSets, E, cache)) {
                    full[set] = 1;
                }
            }
            if (full[set]) {
                evict_and_replace(set, tag, numSets, E, cache);
                if (v) {
                    printf("evict ");
                }
                evict++;
            }
        }

        if (identifier == 'M') {
            if (v) {
                printf("hit ");
            }
            hit++;
        }
        if (v) printf("\n");
        update_LRU(numSets, E, cache);
    }	
    fclose(pFile);	//remember	to	close	file	when	done	
    printSummary(hit, miss, evict);
    return 0;
}
