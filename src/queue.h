#ifndef QUEUE_H
#define QUEUE_H

#include "phone_forward.h"

typedef struct Element {
    struct Backward *bwd;
    struct Element *next;
} Element;

typedef struct Queue {
    struct Element *front;
    struct Element *back;
} Queue;

Queue* queueNew(void);

bool queueIsEmpty(Queue *q);

bool queueAdd(Backward *bwd, Queue *q);

Backward *queueGet(Queue* q);

Backward *queueTop(Queue *q);

void queueDelete(Queue *q);

#endif /* QUEUE_H */