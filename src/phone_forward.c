#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "phone_forward.h"
#include "phnum.h"
#include "utils.h"

/**
 * @brief Pojedynczy wierzchołek drzewa TRIE.
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

    pf->children = calloc(10, sizeof(Node*));
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

static void deleteUpPath(Node *currentNode, size_t addDepth) {
    while (currentNode->depth > addDepth) {
        Node *father = currentNode->father;

        free(currentNode->children);
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
        int digit = num[i] - '0';
        if (pf->children[digit] == NULL) {
            // Ustalamy wysokość, gdzie po raz pierwszy
            // zaczęliśmy dodawać wierzchołki,
            // aby w razie czego wiedzieć dokąd usunąć ścieżkę.
            addDepth = min(addDepth, pf->depth + 1);

            Node *node = phfwdNewNode(digit + '0', pf);
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

/**
 * Dodanie przekierowania wiąże się z odnalezieniem w strukturze
 * wierzchołków reprezentujących @p num1 oraz @p num2
 * i ustawieniem pola @p fwd w pierwszym wierzchołku
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

        node = node->children[num[i] - '0'];
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
/**
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

        if (!phnumAdd(result, copyString(num))) {
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
 * Funkcja zostanie uzupełniona przy kolejnych częściach zadania.
 */
extern PhoneNumbers *phfwdReverse(PhoneForward const *pf, char const *num) {
    pf = NULL;
    num = NULL;
    pf = (PhoneForward const *) num;
    num = (char const *) pf;

    return NULL;
}

/**
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
    // polega na porównaniu czasu dodania przekierowania
    // z największym czasem wyczyszczenia wśród przodków.
    //
    // Dodatkowo następuje zwiększenie czasu struktury.
    pf->time++;
    removeNode->deleteTime = pf->time;
}

/**
 * Fizycznie zwalnia pamięć, która została zaalokowana na strukturę.
 */
extern void phfwdDelete(PhoneForward *pf) {
    if (pf == NULL) {
        return;
    }

    Node *node = pf->rootNode;
    // Sprawdzenie czy wszystkie poddrzewa struktury zostały usunięte.
    while (pf->rootNode->sonsDeleted < 10) {

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

    free(pf->rootNode->children);
    free(pf->rootNode);
    free(pf);
}