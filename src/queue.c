/*
    Implementacja kolejki wskaźnikowej.
*/

#include <stdlib.h>
#include <stdbool.h>

#include "phone_forward.h"
#include "queue.h"

//TODO - zwraca wskaźniki, update komentarzy
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
    Element *tmp = (Element*) malloc(sizeof(Element));
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

// Usunięcie pierwszego elementu z kolejki i zwrócenie jego wartości.
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

// Zwrócenie wartości pierwszego elementu kolejki bez jego usuwania.
extern Backward *queueTop(Queue *q) {
    Backward *bwd = NULL;

    if (!queueIsEmpty(q)) {
        bwd = q->front->bwd;
    }

    return bwd;
}

// Zwolnienie całej pamięci zajmowanej przez kolejkę.
void queueDelete(Queue *q) {
    while (!queueIsEmpty(q)) {
        Element *tmp = q->front;
        q->front = q->front->next;
        free(tmp->bwd);
        free(tmp);
    }

    free(q);
}