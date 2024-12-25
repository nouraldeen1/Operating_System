#include "memory.h"
#include <stdlib.h>
#include <string.h>
FILE *log_file;  // File pointer for logging



// Function to initialize memory
void initialize_memory(memory *mem, int total_size) {
    mem->front = (block *)malloc(sizeof(block));
    mem->rear = mem->front;
    mem->front->size = nearest_power_of_2(total_size);
    mem->front->isused = false;
    mem->front->next = NULL;
    mem->front->id = 0;
    mem->front->start_address = 0;
}

// Function to find the nearest power of 2 greater than or equal to the requested size
int nearest_power_of_2(int size) {
    return (int)pow(2, ceil(log2(size)));
}

// Function to allocate memory
bool allocate_memory(memory *mem, int size, int id,int time) {
    block *current = mem->front;
    int aligned_size = nearest_power_of_2(size);
    int start_address = 0;

    while (current != NULL) {
        if(size==0)
            return true;
        if (!current->isused && current->size >= aligned_size) {
            start_address = current->start_address;
            while (current->size > aligned_size) {
                block *buddy = (block *)malloc(sizeof(block));
                buddy->size = current->size / 2;
                buddy->isused = false;
                buddy->next = current->next;
                current->next = buddy;
                current->size /= 2;
                buddy->start_address = current->start_address + current->size;
            }
            
            current->isused = true;
            current->id = id;
            log_allocation(time, size, id, start_address, start_address + aligned_size - 1);
            return true;
        }
        current = current->next;
    }

    printf("Memory allocation failed. No suitable  available.\n");
    return false;
}

// Function to log memory allocation to file
void log_allocation(int time, int size, int id, int start, int end) {
    log_file = fopen("inputs_logs/memory.log", "a");  // Open the log file for writing
    if (log_file == NULL) {
        printf("Error opening log file.\n");
    }
    fprintf(log_file, "At time %d allocated %d bytes for process %d from %d to %d\n", time, size, id, start, end);
    fclose(log_file);  // Close the log file
}

// Function to merge adjacent free s of the same size
void merge_s(memory *mem) {
    block *current = mem->front;

    while (current != NULL) {
        if (!current->isused) {
            // Calculate the buddy's address
            int buddy_address = (current->start_address % (2 * current->size) == 0)
                                    ? current->start_address + current->size
                                    : current->start_address - current->size;

            // Find the buddy block
            block *buddy = mem->front;
            block *prev = NULL; // Keep track of the previous block
            while (buddy != NULL) {
                if (buddy->start_address == buddy_address && buddy->size == current->size) {
                    break;
                }
                prev = buddy;
                buddy = buddy->next;
            }

            // Merge with buddy if conditions are met
            if (buddy != NULL && !buddy->isused && buddy->size == current->size) {
                if (current->start_address < buddy->start_address) {
                    // Current is the lower block
                    current->size *= 2;
                    current->next = buddy->next;
                    free(buddy);
                } else {
                    // Buddy is the lower block
                    buddy->size *= 2;
                    buddy->next = current->next;
                    if (prev != NULL) {
                        prev->next = buddy;
                    }
                    free(current);
                    current = buddy;
                }
                continue; // Restart from the current block to check for further merges
            }
        }
        current = current->next; // Move to the next block
    }
}


// Function to deallocate memory
bool deallocate_memory(memory *mem, int id,int time,int size) {
    block *current = mem->front;

    while (current != NULL) {
        if (current->isused && current->id == id) {
            int start_address = current->start_address;
            int end_address = start_address + current->size - 1;
            current->isused = false;
            log_deallocation(time,  size, id, start_address, end_address);
            merge_s(mem);
            return true;
        }
        current = current->next;
    }

    printf("Deallocation failed. No  with ID %d found.\n", id);
    return false;
}

// Function to log memory deallocation to file
void log_deallocation(int time, int size, int id, int start, int end) {
    log_file = fopen("inputs_logs/memory.log", "a");  // Open the log file for writing
    if (log_file == NULL) {
        printf("Error opening log file.\n");
    }
    fprintf(log_file, "At time %d freed %d bytes from process %d from %d to %d\n", time, size, id, start, end);
    fclose(log_file);  // Close the log file

}

// Function to print the memory s and their status
void print_memory(memory *mem) {
    block *current = mem->front;
    while (current != NULL) {
        printf(" Size: %d bytes, Status: %s, ID: %d\n", 
               current->size, current->isused ? "Used" : "Free", current->id);
        current = current->next;
    }
    printf("\n");
}

// Function to clean up memory (free all blocks)
void cleanup_memory(memory *mem) {
    block *current = mem->front;
    block *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    mem->front = mem->rear = NULL;
}

/*int main() {
    memory mem;
    log_file = fopen("memory.log", "w");  // Open the log file for writing
    if (log_file == NULL) {
        printf("Error opening log file.\n");
        return 1;
    }

    initialize_memory(&mem, 1024);

    allocate_memory(&mem, 2, 1,7);  // Allocate 200 bytes with ID 1
    print_memory(&mem);

    allocate_memory(&mem, 4, 2,8);  // Allocate 170 bytes with ID 2
    print_memory(&mem);

    deallocate_memory(&mem, 2,4);  // Deallocate  with ID 2
    print_memory(&mem);

    deallocate_memory(&mem, 1,4);  // Deallocate  with ID 1
    print_memory(&mem);

    cleanup_memory(&mem);
    fclose(log_file);  // Close the log file
    

    return 0;
}
*/
