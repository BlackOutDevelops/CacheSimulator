// Joshua Frazer jo227789
// Simulator Cache Project, Spring 2020
// Using Atom & Linux Bash Shell on Windows 10.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#define CACHE_BLOCK_SIZE 64


// int leastRecentlyUsed(L1 *memory);
// int printCache(L1 *memory, int cache);
// int firstInFirstOut();
// int writeThrough();
// int writeBack();
void simulate_access(long long int add);
void update_lru(long long int add);
void update_fifo(long long int add);
void printArray();

int assoc = 30,
    cacheSize,
    numSets,
    set,
    blockOffset = log10(CACHE_BLOCK_SIZE)/log10(2),
    hit = 0,
    miss = 0,
    read,
    write,
    lru,
    wb;
char op;
long long int tag;
long long int **tag_array;
long long int **add_array;
long long int **lru_position;
bool **dirty;

int main(int argc, char **argv)
{
    int i,
        r = 0,
        w = 0;
    double missRatio;
    char buffer[900];
    long long int add;
    FILE *inp = fopen(argv[5], "r");

    cacheSize = atoi(argv[1]);
    assoc = atoi(argv[2]);
    lru = atoi(argv[3]);
    wb = atoi(argv[4]);

    numSets = (cacheSize)/(CACHE_BLOCK_SIZE*assoc);

    // Allocate tag array
    tag_array = (long long int **)calloc(numSets, sizeof(long long int *));
    for (i = 0; i < numSets; i++)
        tag_array[i] = (long long int *)calloc(assoc, sizeof(long long int));

    // Allocate address array
    add_array = (long long int **)calloc(numSets, sizeof(long long int *));
    for (i = 0; i < numSets; i++)
        add_array[i] = (long long int *)calloc(assoc, sizeof(long long int));

    // Allocate dirty array
    dirty = (bool **)calloc(numSets, sizeof(long long int *));
    for (i = 0; i < numSets; i++)
        dirty[i] = (bool *)calloc(assoc, sizeof(long long int));

    // Personal counter
    i = 0;

    // printArray();
    while (fgets(buffer, sizeof(buffer), inp))
    {
        op = buffer[0];
        add = strtoll(buffer + 2, NULL, 0);
        set = (add / CACHE_BLOCK_SIZE) % numSets;
        tag = (double)(add>>blockOffset) / CACHE_BLOCK_SIZE;
        // Simulation begins
        i++;
        if (op == 'R')
            r++;
        else
            w++;

        simulate_access(add>>blockOffset);

        // simulation ends
    }
    fclose(inp);
    // Print statistics
    // printArray();
    printf("Miss Ratio: %.6lf\nWrites to Memory: %d\nReads from Memory: %d\n", missRatio = (double)miss/(miss+hit), write, read);
    // printf("Amount of Read and Write Operations: %d  # of Reads: %d  # of Writes: %d\n", i, r, w);
    // printf("%d\n", blockOffset);

    return 0;
}

void simulate_access(long long int add)
{
    int i,
        flag = 0;
    {
        // Run through array
        for (i = 0; i < assoc; i++)
        {
            // Check if tag is within cache
            if (tag_array[set][i] == tag)
            {
                hit++;
                if (wb == 0 && op == 'W')
                    write++;
                if (lru == 0)
                {
                    flag = 1;
                    update_lru(add);
                    break;
                }
                else
                {
                    flag = 1;
                    update_fifo(add);
                    break;
                }
            }
        }
        // Check if cache was updated or not
        if (flag == 0)
        {
            read++;
            miss++;
            if (wb == 0 && op == 'W')
                    write++;
            if (lru == 0)
            {
                flag = 1;
                update_lru(add);
            }
            else
            {
                flag = 1;
                update_fifo(add);
            }
        }
    }
}

void update_lru(long long int add)
{
    long long int temp;
    bool dcheck = false;
    int i,
        flag,
        location = 0;

    // Perform LRU
    // Find location of address, if it already exists in set
    for (i = 0; i < assoc; i++)
    {
        if (tag == tag_array[set][i])
        {
            temp = tag_array[set][i];
            location = i;
            flag = 1;
            if (dirty[set][i] == true)
                dcheck = true;
        }
    }

    if (flag == 1)
    {
        // Move everything over from the address's location then insert at beginning
        for (i = location; i >= 0; i--)
        {
            // Put tag in front of stack
            if (i == 0)
            {
                if (wb == 1)
                    dirty[set][i] = dcheck;
                if (wb == 1 && op == 'W')
                    dirty[set][i] = true;
                tag_array[set][i] = temp;
                add_array[set][i] = add;
            }
            else
            {
                // Slide tag over by 1
                if (wb == 1)
                    dirty[set][i] = dirty[set][i - 1];
                add_array[set][i] = add_array[set][i - 1];
                tag_array[set][i] = tag_array[set][i - 1];
            }
        }
    }
    else
    {
        // Move everything over then insert at beginning of set
        for (i = assoc-1; i >= 0; i--)
        {
            // Place dirty bit on write
            if (wb == 1 && op == 'W' && i == 0)
                dirty[set][0] = true;
            if (wb == 1 && i == assoc-1 && dirty[set][assoc-1] == true)
                write++;
            if (i == 0)
            {
                if (wb == 1 && op == 'R')
                    dirty[set][i] = false;
                tag_array[set][i] = tag;
                add_array[set][i] = add;
            }
            else
            {
                if (wb == 1)
                    dirty[set][i] = dirty[set][i - 1];
                add_array[set][i] = add_array[set][i - 1];
                tag_array[set][i] = tag_array[set][i - 1];
            }
        }
    }
}

void update_fifo(long long int add)
{
    int i,
        flag = 0;


    // Check if set is full, if it isn't full add address
    for (i = 0; i < assoc; i++)
    {
        if (0 == tag_array[set][i])
        {
            add_array[set][i] = add;
            tag_array[set][i] = tag;
            flag = 1;
            break;
        }
    }

    // If cache is full perform FIFO
    if (flag == 0)
    {
        for (i = 0; i < assoc; i++)
        {
            if (wb == 1 && i == 0 && dirty[set][i] == true)
                write++;
            if (i == assoc-1)
            {
                // Place dirty bit on write
                if (wb == 1 && op == 'W')
                    dirty[set][i] = true;
                else if (wb == 1 && op == 'R')
                    dirty[set][i] = false;
                add_array[set][i] = add;
                tag_array[set][i] = tag;
            }
            else
            {
                dirty[set][i] = dirty[set][i+1];
                add_array[set][i] = add_array[set][i+1];
                tag_array[set][i] = tag_array[set][i+1];
            }
        }
    }
}

void printArray()
{
    int i, j;

    for (i = 0; i < numSets; i++)
        for (j = 0; j < assoc; j++)
            printf("[Set #%d, Tag #%d]: %llx    [Set #%d, Tag #%d]: %lld   [Set #%d, Tag #%d]: %s\n", i + 1, j + 1, add_array[i][j], i + 1, j + 1, tag_array[i][j], i + 1, j + 1, dirty[i][j] ? "True" : "False");
}
