#include "queue.h"

//initialization

Queue_final* createQueue_final() {
    Queue_final* q = (Queue_final*)malloc(sizeof(Queue_final));
    q->front = NULL;
    q->rear = NULL;
    return q;
}

bool isEmpty(Queue_final* q) {
    return q->front == NULL;
}

void enQueue_final(Queue_final* q, struct PCB* pcb) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->pcb = pcb;
    newNode->next = NULL;

    if (isEmpty(q)) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}
struct PCB* frontQueue_final(Queue_final* q) {
    if (isEmpty(q)) {
        return NULL;
    }
    return q->front->pcb;
}

struct PCB* deQueue_final(Queue_final* q) {
    if (isEmpty(q)) {
        
        return NULL;
    }

    Node* temp = q->front;
    struct PCB* pcb = temp->pcb;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    return pcb;
}

void freeQueue_final(Queue_final* q) {
    Node* current = q->front;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp->pcb);
        free(temp);
    }
    free(q);
}

void printQueue_final(Queue_final* q) {
    Node* current = q->front;
    while (current != NULL) {
        printf("PCB ID: %d, PID: %d, State: %s, Arrival Time: %d\n", 
               current->pcb->id, current->pcb->pid, current->pcb->state, current->pcb->arrivalTime);
        current = current->next;
    }
}
