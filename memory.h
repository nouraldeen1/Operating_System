#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct memory {
    struct block *front;
    struct block *rear;
} memory;

typedef struct block {
    int size;
    bool isused;
    struct block *next;
    int id;
    int start_address;  // Start address of the memory 
} block;
void initialize_memory(memory *mem, int total_size);
bool allocate_memory(memory *mem, int size, int id,int time);
bool deallocate_memory(memory *mem, int id,int time,int size);
void print_memory(memory *mem);
void cleanup_memory(memory *mem);
int nearest_power_of_2(int size);
void merge_s(memory *mem);
void log_allocation(int time, int size, int id, int start, int end);
void log_deallocation(int time, int size, int id, int start, int end);
#endif