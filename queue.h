#ifndef Queue_final_H
#define Queue_final_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pcb.h"


typedef struct node {
    struct PCB *pcb;
    struct node *next;
} Node;

typedef struct Queue_final {
    Node *front;
    Node *rear;
} Queue_final;

Queue_final* createQueue_final();
struct PCB* frontQueue_final(Queue_final* q);
bool isEmpty(Queue_final* q);
void enQueue_final(Queue_final* q, struct PCB* pcb);
struct PCB* deQueue_final(Queue_final* q);
void freeQueue_final(Queue_final* q);
void printQueue_final(Queue_final* q);

#endif
