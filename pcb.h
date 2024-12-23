#ifndef PCB_H
#define PCB_H
typedef struct PCB {
    int id;
    int pid;
    char* state;
    int arrivalTime;
    int runTime;
    int priority;
    int waitingTime;
    int remainingTime;
    int startTime;
    int endTime;
    int stoppedTime;
    int memory;
} PCB;
// input from process generator
struct process_input_data
{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
    int mem;
    int remainingTime;
    int startTime;
    int dum; 
    
};

#endif
