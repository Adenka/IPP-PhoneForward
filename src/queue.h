/** @file utils.c 
 * Interfejs kolejki.
 * 
 * @author Jagoda Bobińska (jb438249@students.mimuw.edu.pl)
 * @copyright Uniwersytet Warszawski
 * @date 2022
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "phone_forward.h"

struct Element;
/**
 * Definiuje strukturę Element.
 */
typedef struct Element Element;

struct Queue;
/**
 * Definiuje kolejkę.
 */
typedef struct Queue Queue;

/**
 * @brief Tworzy nową kolejkę.
 * 
 * @return Wskaźnik na nową kolejkę.
 */
Queue* queueNew(void);

/**
 * @brief Sprawdza, czy dana kolejka jest pusta.
 * 
 * @param[in] q - wskaźnik na kolejkę.
 * @return Wartość @p true jeśli kolejka jest pusta,
 *         wartość @p false w przeciwnym wypadku.
 */
bool queueIsEmpty(Queue *q);

/**
 * @brief Dodaje nowy element na kolejkę.
 * 
 * @param[in] bwd - wskaźnik do nowego elementu,
 * który dodajemy na kolejkę (typu Backward);
 * @param[in] q - wskaźnik na kolejkę, na którą dodajemy element.
 * @return Wartość @p true jeśli dodanie elementu się powiodło,
 *         wartość @p false w przeciwnym wypadku.
 */
bool queueAdd(Backward *bwd, Queue *q);

/**
 * @brief Usuwa pierwszy element z kolejki i zwraca jego wartość.
 * 
 * @param[in] q - wskaźnik na kolejkę. 
 * @return Wskaźnik na wartość typu Backward.
 */
Backward *queueGet(Queue* q);

/**
 * @brief Zwraca wartość pierwszego elementu kolejki bez jego usuwania.
 * 
 * @param[in] q - wskaźnik na kolejkę.
 * @return Wskaźnik na wartość typu Backward.
 */
Backward *queueTop(Queue *q);

/**
 * @brief Fizycznie usuwa kolejkę (zwolnienie pamięci).
 * 
 * @param[in] q - wskaźnik na kolejkę do usunięcia.
 */
void queueDelete(Queue *q);

/**
 * @brief Tworzy kopię kolejki.
 * 
 * @param[in] q - kolejka, której kopię tworzymy.
 * @return Wskaźnik do kopii kolejki.
 */
Queue *queueCopy(Queue *q);

#endif /* __QUEUE_H__ */