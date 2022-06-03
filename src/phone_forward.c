/** @file phone_forward.c 
 * Implementacja klasy przechowującej przekierowania numerów telefonicznych.
 * 
 * @author Jagoda Bobińska (jb438249@students.mimuw.edu.pl)
 * @copyright Uniwersytet Warszawski
 * @date 2022
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "phone_forward.h"
#include "phnum.h"
#include "utils.h"
#include "queue.h"

/**
 * @brief Struktura przechowująca informacje na temat przekierowań wstecz.
 */
typedef struct Backward {
    /// Wierzchołek, z którego nastąpiło przekierowanie.
    Node *fwdFrom;

    /// Czas, w którym nastąpiło przekierowanie.
    size_t fwdTime;
} Backward;

extern Backward *makeCopyBackward(Backward *bwd) {
    if (bwd == NULL) {
        return NULL;
    }

    Backward *bwdCopy = malloc(sizeof(Backward));
    if (bwdCopy == NULL) {
        return NULL;
    }

    bwdCopy->fwdFrom = bwd->fwdFrom;
    bwdCopy->fwdTime = bwd->fwdTime;

    return bwdCopy;
}

/**
 * @brief Tworzy nową strukturę typu Backward (przekierowanie wstecz).
 * 
 * @param[in] fwdFrom - wskaźnik do wierzchołka,
 *                      z którego nastąpiło przekierowanie
 * @param[in] fwdTime - czas kiedy nastąpiło przekierowanie
 * @return Nowa struktura typu Backward.
 */
static Backward* backwardNew(Node *fwdFrom, size_t fwdTime) {
    Backward *backward = malloc(sizeof(Backward));
    if (backward ==  NULL) {
        return NULL;
    }

    if (fwdFrom == NULL) {
        return NULL;
    }
    
    backward->fwdFrom = fwdFrom;
    backward->fwdTime = fwdTime;

    return backward;
}

/**
 * @brief Pojedynczy wierzchołek drzewa TRIE.
 * Właściwa struktura przechowująca informację dotyczące numerów i przekierowań.
 */
typedef struct Node {
    /// 12-elementowa tablica - reprezentuje kolejne cyfry numeru.
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

    /// Kolejka wierzchołków, z których istnieje przekierowanie
    /// do danego wierzchołka.
    Queue* backwards;

} Node;

/**
 * Struktura przechowująca przekierowania numerów telefonów.
 *
 * Struktura składa się z wierzchołka nadrzędnego,
 * który zawiera przekierowanie do właściwego korzenia TRIE (już typu Node)
 * i czas struktury
 * (potrzebny do określania kolejności dodawania przekierowań i ich usuwania).
 */
struct PhoneForward {
    /// Korzeń TRIE.
    Node *rootNode;
    /// Czas.
    size_t time;
};

/**
 * @brief Utworzenie nowego wierzchołka drzewa i inicjalizacja paramterów.
 * 
 * @param[in] digit - cyfra, którą ma reprezentować inicjaliowany wierzchołek;
 * @param[in] father - ojciec inicjalizowanego wierzchołka.
 * @return Zainicjalizowany wierzchołek.
 */
static Node *phfwdNewNode(char digit, Node *father) {
    Node *pf = malloc(sizeof(Node));
    if (pf == NULL) {
        return NULL;
    }

    pf->children = calloc(12, sizeof(Node*));
    if (pf->children == NULL) {
        free(pf);
        return NULL;
    }

    pf->father = father;
    pf->fwd = NULL;

    pf->digit = digit;
    pf->depth = (father == NULL ? 0 : father->depth + 1);

    pf->fwdTime = 0;
    pf->deleteTime = 0;

    pf->sonsDeleted = 0;
    
    pf->backwards = queueNew();

    if (pf->backwards == NULL) {
        free(pf->children);
        free(pf);

        return NULL;
    }

    return pf;
}

extern PhoneForward *phfwdNew(void) {
    PhoneForward *pf = malloc(sizeof(PhoneForward));
    if (pf == NULL) {
        return NULL;
    }

    pf->time = 0;

    // Ustawienie w korzeniu znaku niebędącego cyfrą (atrapa).
    Node *rootNode = phfwdNewNode('a', NULL);
    if (rootNode == NULL) {
        free(pf);
        return NULL;
    }

    pf->rootNode = rootNode;

    return pf;
}

/**
 * @brief Usunięcie utworzonych wierzchołków przy błędzie alokacji pamięci.
 * 
 * Jeżeli podczas dodawania nowego wierzchołka wystąpi błąd alokacji pamięci,
 * to usuwamy nowoutworzoną ścieżkę.
 * 
 * @param[in] currentNode - wierzchołek,
 *                          który udało nam się zaalokować jako ostatni
 * @param[in] addDepth - głębokość, na której kończymy usuwanie;
 *                       (dotarliśmy do początku nowoutworzonej ścieżki).
 */
static void deleteUpPath(Node *currentNode, size_t addDepth) {
    while (currentNode->depth > addDepth) {
        Node *father = currentNode->father;

        father->children[toInt(currentNode->digit)] = NULL;
        free(currentNode->children);
        queueDelete(currentNode->backwards);
        free(currentNode);

        currentNode = father;
    }
}

/**
 * @brief Znajduje wierzchołek reprezentujący podany napis.
 * 
 * Iteracyjnie przechodzimy od korzenia do wierzchołka reprezentującego @p num;
 * jeśli taki wierzchołek nie istnieje, na bieżąco tworzymy do niego ścieżkę.
 * 
 * W razie niepowodzenia (błąd alokacji pamięci)
 * dotychczas utworzona ścieżka zostaje usunięta.
 * 
 * @param[in, out] pf - wskaźnik do struktury przechowującej przekierowania
 *                numerów telefonów;
 * @param[in] num - numer telefonu, który mamy znaleźć;
 * @param[in] length - długość numeru.
 * @return Szukany wierzchołek
 */
static Node *phfwdFind(Node *pf, char const *num, size_t length) {
    if (pf == NULL || !ifNumOk(num)) {
        return NULL;
    }

    size_t addDepth = SIZE_MAX;
    for (size_t i = 0; i < length; ++i) {
        int digit = toInt(num[i]);
        if (pf->children[digit] == NULL) {
            // Ustalamy wysokość, gdzie po raz pierwszy
            // zaczęliśmy dodawać wierzchołki,
            // aby w razie czego wiedzieć dokąd usunąć ścieżkę.
            addDepth = min(addDepth, pf->depth + 1);

            Node *node = phfwdNewNode(toChar(digit), pf);
            // Usunięcie ścieżki w razie niepowodzenia alokacji pamięci.
            if (node == NULL) {
                deleteUpPath(pf, addDepth);
                return NULL;
            }

            pf->children[digit] = node;
        }

        pf = pf->children[digit];
    }

    return pf;    
}

/*
 * Dodanie przekierowania wiąże się z odnalezieniem w strukturze
 * wierzchołków reprezentujących num1 oraz num2
 * i ustawieniem pola fwd w pierwszym wierzchołku
 * jako wskaźnik na drugi wierzchołek.
 */
extern bool phfwdAdd(PhoneForward *pf, char const *num1, char const *num2) {
    if (!ifNumOk(num1) || !ifNumOk(num2) || pf == NULL 
                       || num1 == num2   || strcmp(num1, num2) == 0) {
        return false;
    }

    Node *num1Node = phfwdFind(pf->rootNode, num1, stringLength(num1));
    Node *num2Node = phfwdFind(pf->rootNode, num2, stringLength(num2));
    if (num1Node == NULL || num2Node == NULL) {
        return false;
    }

    pf->time++;

    num1Node->fwd = num2Node;
    num1Node->fwdTime = pf->time;
    
    Backward *tmp = backwardNew(num1Node, pf->time);
    if (!queueAdd(tmp, num2Node->backwards)) {
        free(tmp);
        return false;
    }

    return true;
}

/**
 * @brief Zwrócenie numeru reprezentowanego przez dany wierzchołek.
 * Algorytm polega na przejściu od @p node do korzenia na podstawie @p father.
 * Dzięki parametrowi @p depth znamy końcową długość napisu
 * i możemy od razu uzupełniać wynikową tablicę.
 * @param[in] node - wierzchołek, do którego mamy znaleźć odpowiadający numer.
 * @return Znaleziony numer.
 */
static char *phfwdRead(Node *node) {
    if (node == NULL) {
        return NULL;
    }

    size_t depth = node->depth;
    char *result = malloc(sizeof(char) * (depth + 1));
    if (result == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < depth; ++i) {
        result[depth - i - 1] = node->digit;
        node = node->father;
    }

    result[depth] = '\0';
    
    return result;
}

/**
 * @brief Znalezienie ostatniego przekierowania od korzenia do wierzchołka.
 * Algorytm wyszukania tego wierzchołka polega na przejściu ścieżki
 * od korzenia do wierzchołka reprezentującego @p num i
 * znalezieniu dzięki temu ostatniego wierzchołka zawierającego przekierowanie.
 * @param[in] node - wierzchołek początkowy;
 * @param[in] num - zadany numer
 * @return Wierzchołek reprezentujący najdłuższy możliwy prefiks numeru,
 *         z którego istnieje przekierowanie.
 */
static Node *phfwdFindLastFwd(Node *node, char const *num) {
    if (node == NULL || !ifNumOk(num)) {
        return NULL;
    }

    Node *result = NULL;
    size_t depth = stringLength(num);
    size_t maxTime = 0;

    for (size_t i = 0; i < depth && node != NULL; ++i) {
        maxTime = max(maxTime, node->deleteTime);

        if (node->fwd != NULL && node->fwdTime > maxTime) {
            result = node;
        }

        node = node->children[toInt(num[i])];
    }

    // Sprawdzenie ostatniego wierzchołka
    // (na głębokości równej długości napisu)
    if (node != NULL) {
        maxTime = max(maxTime, node->deleteTime);

        if (node->fwd != NULL && node->fwdTime > maxTime) {
            result = node;
        }
    }

    return result;
}

/**
 * @brief Konstrukcja wyniku funkcji phfwdGet.
 * Konstrukcja składa się z określenia na co przekierowujemy zadany numer
 * (prefix wyniku) oraz uzupełnienia wyniku resztą oryginalnego numeru.
 * @param[in] num - numer, z którego wykonujemy przekierowanie;
 * @param[in] NumLastFwd - wierzchołek zawierający ostatnie przekierowanie
 *                     na trasie od korzenia do wierzchołka
 *                     reprezentującego @p num.
 * @return Przekierowany napis.
 */
static char *constructResultString(char const *num, Node *NumLastFwd) {
    if (num == NULL || NumLastFwd == NULL) {
        return NULL;
    }

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

/*
 * Wynikowa struktura będzie składać się z maksymalnie jednego numeru
 * (z każdego wierzchołka możemy mieć tylko jedno przekierowanie).
 * 
 * Konstruujemy wynik, a następnie (jeśli przekierowanie istnieje)
 * dodajemy go na wynikową strukturę i ją zwracamy.
 */
extern PhoneNumbers *phfwdGet(PhoneForward const *pf, char const *num) {
    if (pf == NULL) {
        return NULL;
    }

    if (!ifNumOk(num)) {
        return phnumNew();
    }

    Node *NumLastFwd = phfwdFindLastFwd(pf->rootNode, num);

    if (NumLastFwd == NULL) {
        PhoneNumbers *result = phnumNew();
        if (result == NULL) {
            return NULL;
        }

        char *copy = copyString(num);
        if (!phnumAdd(result, copy)) {
            free(copy);
            free(result);
            return NULL;
        }

        return result;
    }

    char *resultString = constructResultString(num, NumLastFwd);
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

/**
 * @brief Sprawdza czy przekierowanie jest aktualne.
 * 
 * Sprawdzamy wszystkie wierzchołki na trasie od wierzchołka,
 * z którego wyszło przekierowanie do korzenia
 * i sprawdzamy czy poddrzewo, w którym się znajdujemy był czyszczony.
 * 
 * @param[in] bwd - struktura Backward (określa badane przekierowane).
 * @return Wartość @p true jeśli przekierowanie jest aktualne,
 *         wartość @p false w przeciwnym wypadku.
 */
static bool isNotClearedBefore(Backward *bwd) {
    if (bwd == NULL || bwd->fwdTime < bwd->fwdFrom->fwdTime) {
        return false;
    }

    Node *currentNode = bwd->fwdFrom;

    while (currentNode != NULL) {
        if (currentNode->deleteTime > bwd->fwdTime) {
            return false;
        }

        currentNode = currentNode->father;
    }
    
    return true;
}

/**
 * @brief Skonstruuje końcowy napis funkcji phfwdReverse.
 * 
 * @param[in] num - sufiks oryginalnego napisu;
 * @param[in] fwdFrom - wierzchołek reprezentujący numer,
 * na który przekierowany został prefiks oryginalnego numeru.
 * @return Połącznie dwóch napisów (num i tego reprezentowanego przez fwdFrom),
 *         wartość @p NULL jeśli dodanie się nie udało.
 */
static char *constructResultStringRev(char const *num, Node *fwdFrom) {
    if (num == NULL || fwdFrom == NULL) {
        return NULL;
    }

    // Początkowa wartość wyniku - oryginalne przekierowanie.
    char *resultString = phfwdRead(fwdFrom);

    if (resultString == NULL) {
        return NULL;
    }

    size_t fwdLength = fwdFrom->depth;
    size_t numLength = stringLength(num) - 1;

    char *backup = resultString;
    // Zaalokowanie pamięci potrzebnej na wynik
    // (możemy określić jego długość na podstawie znanych parametrów).
    resultString = realloc(resultString, sizeof(char) *
                          (fwdLength + numLength + 1));
    if (resultString == NULL) {
        free(backup);
        return NULL;
    }

    // Przepisanie reszty numeru do ostatecznego wyniku.
    for (long long i = 0; i <= (long long) numLength - 1; ++i) {
        resultString[fwdLength + i] = num[i + 1];
    }

    resultString[fwdLength + numLength] = '\0';

    return resultString;
}

/**
 * @brief Rozpatrzenie przekierowań na jeden prefiks oryginalnego numeru.
 * 
 * @param[in, out] pnumResult - wskaźnik do wynikowej struktury
 * (na nią wrzucamy znalezione numery)
 * @param[in] currentNode - wierzchołek reprezentujący dany prefiks
 * @param[in] num - sufiks oryginalnego numeru
 * @return Wartość @p true jeśli nie wystąpił błąd alokacji pamięci,
 *         wartość @p false w przeciwnym wypadku.
 */
static bool lookBackwards(PhoneNumbers *pnumResult,
                          Node *currentNode, char const *num) {
    if (pnumResult == NULL || currentNode == NULL || num == NULL)  {
        return false;
    }

    Queue *q = currentNode->backwards;
    // Nowa kolejka - w trakcie przeglądania przekierowań wstecz
    // będziemy przepisywać przekierowania wstecz z oryginalnej kolejki
    // pomijając te, które nie są już aktualne.
    Queue *newQ = queueNew();
    Queue *copyQ = queueCopy(q);

    if (newQ == NULL || copyQ == NULL) {
        queueDelete(newQ);
        queueDelete(copyQ);
        return false;
    }

    while (!queueIsEmpty(copyQ)) {
        Backward *bwd = queueGet(copyQ);
        
        if (isNotClearedBefore(bwd)) {
            char *resultString = constructResultStringRev(num, bwd->fwdFrom);
            if (!phnumAdd(pnumResult, resultString)) {
                free(bwd);
                queueDelete(newQ);
                queueDelete(copyQ);
                free(resultString);

                return false;
            }

            if (!queueAdd(bwd, newQ)) {
                free(bwd);
                queueDelete(newQ);
                queueDelete(copyQ);

                return false;
            }
        }
        else {
            free(bwd);
        }
    }

    currentNode->backwards = newQ;
    queueDelete(copyQ);
    queueDelete(q);

    return true;
}

extern PhoneNumbers *phfwdReverse(PhoneForward const *pf, char const *num) {
    if (pf == NULL) {
        return NULL;
    }

    if (!ifNumOk(num)) {
        return phnumNew();
    }

    Node *currentNode = pf->rootNode->children[toInt(*num)];
    
    PhoneNumbers *pnumResult = phnumNew();
    if (pnumResult == NULL) {
        return NULL;
    }

    if (!phnumAdd(pnumResult, copyString(num))) {
        phnumDelete(pnumResult);
        return NULL;
    }

    // Przeglądanie prefiksów dopóki ciąg znaków się nie skończy.
    while (currentNode != NULL && *num != '\0') {
        // Sprawdzenie danego prefiksu.
        if(!lookBackwards(pnumResult, currentNode, num)) {
            phnumDelete(pnumResult);
            return NULL;
        }
        
        // Skrócenie napisu.
        num += sizeof(char);
        
        // Sprawdzenie, czy ciąg znaków nie skończył się podczas przesunięcia.
        if (*num != '\0') {
            // Sprawdzenie, czy istnieje wierzchołek,
            // do którego chcemy przejść istnieje.
            if (currentNode->children[toInt(*num)] != NULL) {
                currentNode = currentNode->children[toInt(*num)];
            }
            else {
                break;
            }
        }
    }
    
    phnumSort(pnumResult); 

    return phnumRemoveDuplicates(pnumResult);
}

/*
 * Usunięcie poddrzewa nie polega na fizycznym usunięciu poddrzewa
 * (ani przekierowań z niego wychodzących),
 * lecz na ustawieniu w wierzchołku czasu jego wyczyszczenia.
 */
extern void phfwdRemove(PhoneForward *pf, char const *num) {
    if (!ifNumOk(num) || pf == NULL) {
        return;
    }

    Node *removeNode = phfwdFind(pf->rootNode, num, stringLength(num));
    if (removeNode == NULL) {
        return;
    }

    // Późniejsze określenie czy dane przekierowanie zostało usunięte czy nie
    // polega na porównaniu czasux dodania przekierowania
    // z największym czasem wyczyszczenia wśród przodków.
    //
    // Dodatkowo następuje zwiększenie czasu struktury.
    pf->time++;
    removeNode->deleteTime = pf->time;
}

/*
 * Fizycznie zwalnia pamięć, która została zaalokowana na strukturę.
 */
extern void phfwdDelete(PhoneForward *pf) {
    if (pf == NULL) {
        return;
    }

    Node *node = pf->rootNode;
    // Sprawdzenie czy wszystkie poddrzewa struktury zostały usunięte.
    while (pf->rootNode->sonsDeleted < 12) {

        // Sprawdzenie czy wszystkie poddrzewa wierzchołka
        // zostały usunięte.
        if (node->sonsDeleted == 12) {

            // Zwolnienie pamięci, powrót do ojca
            // oraz zwiększenie liczby usuniętych poddrzew.
            free(node->children);
            queueDelete(node->backwards);
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

    free(pf->rootNode->children);
    queueDelete(pf->rootNode->backwards);
    free(pf->rootNode);
    free(pf);
}