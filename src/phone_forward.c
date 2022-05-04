#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "phone_forward.h"

/**
 * @brief Struktura przechowująca ciąg numerów telefonów.
 * Składa się z maksymalnego możliwego rozmiaru struktury,
 * liczby elementów na niej i faktycznego ciągu elementów.
 */
struct PhoneNumbers {
    /// Maksymalna pojemność struktury (początkowo 8).
    size_t size;
    /// Liczba elementów na strukturze.
    size_t count;
    /// Elementy na strukturze.
    char **numbers;
};

/**
 * @brief Sprawdzenie czy numer jest poprawny.
 * 
 * @param[in] num - numer, którego poprawność mamy ocenić;
 * @return Wartość @p true jeśli numer poprawny,
 *          wartość @p false - numer jest NULLem,
 *          jest pustym napisem lub zawiera znaki inne niż cyfry
 */
bool ifNumOk(char const *num) {
    if (num == NULL || *num == '\0') {
        return false;
    }

    num = (char *)num;
    while ('0' <= *num && *num <= '9') {
        num += sizeof(char);
    }

    if (*num == '\0') {
        return true;
    }

    return false;
}

/**
 * @brief Utworzenie nowej struktury przechowywującej ciąg numerów telefonów.
 * 
 * @return Wskaźnik na nową strukturę lub NULL jeśli alokacja pamięci się nie powiodła.
 */
PhoneNumbers *phnumNew(void) {
    PhoneNumbers *phnum = malloc(sizeof(PhoneNumbers));
    if (phnum == NULL) {
        return NULL;
    }

    phnum->size = 8;
    phnum->count = 0;

    phnum->numbers = malloc(8 * sizeof(char*));
    if (phnum->numbers == NULL) {
        free(phnum);
        return NULL;
    }
    
    return phnum;
}

/**
 * @brief Dodanie nowego numeru na strukturę przechowującą ciąg numerów telefonów.
 * 
 * @param[in, out] phnum - struktura przechowująca ciąg numerów telefonów;
 * @param[in] num - numer, który mamy dodać na strukturę.
 * @return Wartość @p true jeśli dodanie się powiodło,
 *          @p false w przeciwnym wypadku.
 */
bool phnumAdd(PhoneNumbers *phnum, char *num) {
    if (phnum == NULL || !ifNumOk(num)) {
        return false;
    }

    // Dynamiczna alokacja pamięci na kolejne elementy tablicy.
    if (phnum->size == phnum->count) {
        phnum->size *= 2;

        char **backup = phnum->numbers;
        size_t count = phnum->count;
        phnum->numbers = realloc(phnum->numbers, sizeof(char*) * phnum->size);
        if (phnum->numbers == NULL) {
            for (size_t i = 0; i<count; ++i) {
                free(backup[i]);
            }
            return false;
        }
        free(backup);
    }

    phnum->numbers[(phnum->count)++] = num;

    return true;
}

char const *phnumGet(PhoneNumbers const *phnum, size_t id) {
    if (phnum == NULL || id >= phnum->count) {
        return NULL;
    }
           
    return phnum->numbers[id];
}

void phnumDelete(PhoneNumbers *phnum) {
    if (phnum != NULL) {
        for (size_t i = 0; i < phnum->count; ++i) {
            free(phnum->numbers[i]);
        }

        free(phnum->numbers);
        free(phnum);
    }
}

/**
 * @brief Pojedynczy wierzchołek drzewa trie.
 * Właściwa struktura przechowująca informację dotyczące numerów i przekierowań.
 */
typedef struct Node {
    /// 10-elementowa tablica - reprezentuje kolejne cyfry numeru.
    struct Node **children;
    /// Poprzednia cyfra numeru.
    struct Node *father;
    /// Przekierowanie z wierzchołka
    /// (prefiksu reprezentowanego przez trasę od korzenia do wierzchołka).
    struct Node *fwd;

    /// Cyfra, którą reprezentuje dany wierzchołek drzewa.
    char digit;
    /// Głębokość, na której znajduje się wierzchołek (równoważne pozycji,
    /// na której w numerze występuje dana cyfra).
    size_t depth;

    /// Czas, kiedy dodane zostało aktualne przekierowanie.
    size_t fwdTime;
    /// Czas, kiedy przekierowania w poddrzewie zostały wyczyszczone.
    size_t deleteTime;

    /// Liczba usuniętych poddrzew. 
    uint8_t sonsDeleted;
} Node;

/**
 * @brief Struktura przechowująca przekierowania numerów telefonów.
 * Struktura składa się z wierzchołka nadrzędnego,
 * który zawiera przekierowanie do właściwego korzenia trie (już typu Node)
 * i czas struktury.
 * (potrzebny do określania kolejności dodawania przekierowań i ich usuwania).
 */
struct PhoneForward {
    /// Korzeń trie.
    Node *rootNode;
    /// Czas.
    size_t time;
};

/**
 * @brief Utworzenie nowego wierzchołka drzewa i inicjalizacja paramterów.
 * 
 * @param digit - cyfra, którą ma reprezentować inicjaliowany wierzchołek;
 * @param father - ojciec inicjalizowanego wierzchołka.
 * @return Zainicjalizowany wierzchołek.
 */
Node *phfwdNewNode(char digit, Node *father) {
    Node *phfwd = malloc(sizeof(Node));
    if (phfwd == NULL) {
        return NULL;
    }

    phfwd->children = calloc(10, sizeof(Node*));
    if (phfwd->children == NULL) {
        free(phfwd);
        return NULL;
    }

    phfwd->father = father;
    phfwd->fwd = NULL;

    phfwd->digit = digit;
    phfwd->depth = (father == NULL ? 0 : father->depth + 1);

    phfwd->fwdTime = 0;
    phfwd->deleteTime = 0;

    phfwd->sonsDeleted = 0;

    return phfwd;
}

PhoneForward *phfwdNew(void) {
    PhoneForward *phfwd = malloc(sizeof(PhoneForward));
    if (phfwd == NULL) {
        return NULL;
    }

    phfwd->time = 0;

    // Ustawienie w korzeniu znaku niebędącego cyfrą (atrapa).
    Node *rootNode = phfwdNewNode('a', NULL);
    if (rootNode == NULL) {
        free(phfwd);
        return NULL;
    }

    phfwd->rootNode = rootNode;

    return phfwd;
}

/**
 * @brief Zwaraca długość poprawnego napisu.
 * 
 * @param string - napis, którego długość mamy określić.
 * @return Długość napisu.
 */
size_t stringLength(char const *string) {
    size_t length = 0;

    while ('0' <= *string && *string <= '9') {
        string += sizeof(char);
        length++;
    }

    return length;
}

/**
 * @brief Znajduje wierzchołek reprezentujący podany napis.
 *        Algorytm wyszukiwania jest iteracyjny i w razie potrzeby
 *        tworzy wierzchołek reprezentujący dany numer.
 * @param phfwd - wskaźnik do struktury przechowującej przekierowania
 *                numerów telefonów;
 * @param num - numer telefonu, który mamy znaleźć;
 * @param length - długość numeru.
 * @return Szukany wierzchołek
 */
Node *phfwdFind(Node *phfwd, char const *num, size_t length) {
    if (phfwd == NULL || !ifNumOk(num)) {
        return NULL;
    }

    for (size_t i = 0; i<length; ++i) {
        int digit = num[i] - '0';
        if (phfwd->children[digit] == NULL) {
            Node *node = phfwdNewNode(digit + '0', phfwd);
            if (node == NULL) {
                return NULL;
            }

            phfwd->children[digit] = node;
        }

        phfwd = phfwd->children[digit];
    }

    return phfwd;    
}

bool phfwdAdd(PhoneForward *phfwd, char const *num1, char const *num2) {
    if (!ifNumOk(num1) || !ifNumOk(num2) || phfwd == NULL || num1 == num2 || strcmp(num1, num2) == 0) {
        return false;
    }

    Node *num1Node = phfwdFind(phfwd->rootNode, num1, stringLength(num1));
    Node *num2Node = phfwdFind(phfwd->rootNode, num2, stringLength(num2));
    if (num1Node == NULL || num2Node == NULL) {
        return false;
    }

    phfwd->time++;

    num1Node->fwd = num2Node;
    num1Node->fwdTime = phfwd->time;

    return true;
}

/**
 * @brief Zwrócenie numeru reprezentowanego przez dany wierzchołek.
 
 * @param node - wierzchołek, do którego mamy znaleźć odpowiadający numer.
 * @return Znaleziony numer.
 */
char *phfwdRead(Node *node) {
    if (node == NULL) {
        return NULL;
    }

    size_t depth = node->depth;
    char *result = malloc(sizeof(char) * depth);
    if (result == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < depth; ++i) {
        result[depth - i - 1] = node->digit;
        node = node->father;
    }
    
    return result;
}

/**
 * @brief Znalezienie ostatniego przekierowania od korzenia do wierzchołka.
 * 
 * @param node - wierzchołek początkowy;
 * @param num - zadany numer
 * @return Wierzchołek reprezentujący najdłuższy możliwy prefiks numeru,
 *         z którego istnieje przekierowanie.
 */
Node *phfwdFindLastFwd(Node *node, char const *num) {
    if (node == NULL || !ifNumOk(num)) {
        return NULL;
    }

    Node *result = NULL;
    size_t depth = stringLength(num);
    size_t maxTime = 0;

    for (size_t i = 0; i < depth && node != NULL; ++i) {
        maxTime = (maxTime < node->deleteTime ? node->deleteTime : maxTime);
        
        if (node->fwd != NULL && node->fwdTime > maxTime) {
            result = node;
        }

        node = node->children[num[i] - '0'];
    }

    // Sprawdzenie ostatniego wierzchołka
    // (na głębokości równej długości napisu)
    if (node != NULL) {
        maxTime = (maxTime < node->deleteTime ? node->deleteTime : maxTime);

        if (node->fwd != NULL && node->fwdTime > maxTime) {
            result = node;
        }
    }

    return result;
}

/**
 * @brief Utworzenie płytkiej kopii zadanego napisu.
 * 
 * @param num - Zadany napis (numer).
 * @return Płytka kopia zadanego napisu (numeru).
 */
char *copyString(char const *num) {
    if (!ifNumOk(num)) {
        return NULL;
    }

    size_t length = stringLength(num);
    char *copy = malloc((length + 1) * sizeof(char));
    if (copy == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < length; ++i) {
        copy[i] = num[i];
    }

    copy[length] = '\0';

    return copy;
}

/**
 * @brief Konstrukcja wyniku funkcji phfwdGet.
 * 
 * @param num - numer, z którego wykonujemy przekierowanie;
 * @param NumLastFwd - wierzchołek zawierający ostatnie przekierowanie
 *                     na trasie od korzenia do wierzchołka
 *                     reprezentującego @p num.
 * @return Przekierowany napis.
 */
char *constructResultString(char const *num, Node *NumLastFwd) {
    // Początkowa wartość wyniku - ostatnie możliwe przekierowanie numeru.
    char *resultString = phfwdRead(NumLastFwd->fwd);
    if (resultString == NULL) {
        return NULL;
    }

    size_t fwdLength = NumLastFwd->fwd->depth;
    size_t lastFwdLength = NumLastFwd->depth;
    size_t numLength = stringLength(num);

    char *backup = resultString;
    // Zaalokowanie pamięci potrzebnej na wynik
    // (możemy określić jego długość na podstawie znanych parametrów).
    resultString = realloc(resultString, sizeof(char) *
                          (fwdLength - lastFwdLength + numLength + 1));
    if (resultString == NULL) {
        free(backup);
        return NULL;
    }

    // Przepisanie reszty numeru do ostatecznego wyniku.
    for (size_t i = 0; i < numLength - lastFwdLength; ++i) {
        resultString[fwdLength + i] = num[lastFwdLength + i];
    }

    resultString[fwdLength - lastFwdLength + numLength] = '\0';

    return resultString;
}

PhoneNumbers *phfwdGet(PhoneForward const *phfwd, char const *num) {
    if (phfwd == NULL) {
        return NULL;
    }

    if (!ifNumOk(num)) {
        return phnumNew();
    }

    Node *NumLastFwd = phfwdFindLastFwd(phfwd->rootNode, num);

    if (NumLastFwd == NULL) {
        PhoneNumbers *result = phnumNew();
        if (result == NULL) {
            return NULL;
        }

        if (!phnumAdd(result, copyString(num))) {
            free(result);
            return NULL;
        }

        return result;
    }

    char const *resultString = constructResultString(num, NumLastFwd);
    PhoneNumbers *result = phnumNew();
    if (result == NULL) {
        free(resultString);
        return NULL;
    }

    if (!phnumAdd(result, resultString)) {
        free(resultString);
        phnumDelete(result);
        return NULL;
    }

    return result;
}

PhoneNumbers *phfwdReverse(PhoneForward const *pf, char const *num) {
    pf = NULL;
    num = NULL;
    pf = (PhoneForward const *) num;
    num = (char const *) pf;

    return NULL;
}

void phfwdRemove(PhoneForward *phfwd, char const *num) {
    if (!ifNumOk(num) || phfwd == NULL) {
        return;
    }

    Node *removeNode = phfwdFind(phfwd->rootNode, num, stringLength(num));
    if (removeNode == NULL) {
        return;
    }

    // Określenie czy dane przekierowanie zostało usunięte czy nie
    // polega na porównaniu czasu dodania przekierowania
    // z największym czasem wyczyszczenia wśród przodków.
    //
    // Dodatkowo następuje zwiększenie czasu struktury.
    phfwd->time++;
    removeNode->deleteTime = phfwd->time;
}

void phfwdDelete(PhoneForward *phfwd) {
    if (phfwd == NULL) {
        return;
    }

    Node *node = phfwd->rootNode;
    // Sprawdzenie czy wszystkie poddrzewa struktury zostały usunięte.
    while (phfwd->rootNode->sonsDeleted < 10) {

        // Sprawdzenie czy wszystkie poddrzewa wierzchołka
        // zostały usunięte.
        if (node->sonsDeleted == 10) {

            // Zwolnienie pamięci, powrót do ojca
            // oraz zwiększenie liczby usuniętych poddrzew.
            free(node->children);
            Node *backup = node->father;
            free(node);
            node = backup;
            node->sonsDeleted++;
        }
        // Przejście do syna w celu jego usunięcia.
        else if (node->children[node->sonsDeleted] != NULL) {
            node = node->children[node->sonsDeleted];
        }
        // Zwiększenie liczby usuniętych poddrzew.
        else {
            node->sonsDeleted++;
        }
    }

    free(phfwd->rootNode->children);
    free(phfwd->rootNode);
    free(phfwd);
}