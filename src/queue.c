/** @file utils.c 
 * Implementacja kolejki.
 * 
 * @author Jagoda Bobińska (jb438249@students.mimuw.edu.pl)
 * @copyright Uniwersytet Warszawski
 * @date 2022
 */

#include <stdlib.h>
#include <stdbool.h>

#include "phone_forward.h"
#include "queue.h"

/**
 * @brief Element kolejki wskaźnikowej.
 */
typedef struct Element {
    /// Przekierowanie wstecz (Backward).
    struct Backward *bwd;

    /// Wskaźnik na kolejny element kolejki.
    struct Element *next;
} Element;

/**
 * @brief Kolejka wskaźnikowa.
 */
typedef struct Queue {
    /// Wskaźnik na początek kolejki.
    struct Element *front;

    /// Wskaźnik na koniec kolejki.
    struct Element *back;
} Queue;

extern Queue *queueNew() {
    Queue *q = malloc(sizeof(Queue));

    if (q == NULL) {
        return NULL;
    }

    q->front = NULL;
    q->back = NULL;

    return q;
}

extern bool queueIsEmpty(Queue *q) {
    if (q == NULL) {
        return true;
    }

    return (q->front == NULL);
}

extern bool queueAdd(Backward* bwd, Queue *q) {
    if (bwd == NULL || q == NULL) {
        return false;
    }

    Element *tmp = malloc(sizeof(Element));
    if (tmp == NULL) {
        return false;
    }

    tmp->bwd = bwd;
    tmp->next = NULL;
    if (!queueIsEmpty(q)) {
        q->back->next = tmp;
        q->back = tmp;
    }
    else {
        q->front = tmp;
        q->back = tmp;
    }

    return true;
}

extern Backward *queueGet(Queue *q) {
    Backward *bwd = NULL;

    if (!queueIsEmpty(q)) {
        Element *tmp = q->front;

        bwd = q->front->bwd;
        q->front = q->front->next;
        free(tmp);
    }

    return bwd;
}

extern Backward *queueTop(Queue *q) {
    Backward *bwd = NULL;

    if (!queueIsEmpty(q)) {
        bwd = q->front->bwd;
    }

    return bwd;
}

void queueDelete(Queue *q) {
    while (!queueIsEmpty(q)) {
        Element *tmp = q->front;
        q->front = q->front->next;
        free(tmp->bwd);
        free(tmp);
    }

    free(q);
}

extern Queue* queueCopy(Queue *q) {
    Queue *queueResult = queueNew();
    if (queueResult == NULL) {
        return NULL;
    }

    Element *queueElement = q->front;
    while (queueElement != NULL) {
        Backward *bwd = makeCopyBackward(queueElement->bwd);

        if (bwd == NULL) {
            queueDelete(queueResult);
            return NULL;
        }

        if (!queueAdd(bwd, queueResult)) {
            free(bwd);
            queueDelete(queueResult);
            return NULL;
        }

        queueElement = queueElement->next;
    }
    
    return queueResult;
}