/** @file utils.c 
 * Implementacja struktury przechowującej ciąg numerów telefonów.
 * 
 * @author Jagoda Bobińska (jb438249@students.mimuw.edu.pl)
 * @copyright Uniwersytet Warszawski
 * @date 2022
 */

#include <string.h>
#include <stdlib.h>

#include "phone_forward.h"
#include "utils.h"

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

extern PhoneNumbers *phnumNew(void) {
    PhoneNumbers *pnum = malloc(sizeof(PhoneNumbers));
    if (pnum == NULL) {
        return NULL;
    }

    pnum->size = 8;
    pnum->count = 0;

    pnum->numbers = malloc(8 * sizeof(char*));
    if (pnum->numbers == NULL) {
        free(pnum);
        return NULL;
    }
    
    return pnum;
}

extern size_t getCount(PhoneNumbers *pnum) {
    return pnum->count;
}

extern bool phnumAdd(PhoneNumbers *pnum, char *num) {
    if (pnum == NULL || !ifNumOk(num)) {
        return false;
    }

    // Dynamiczna alokacja pamięci na kolejne elementy tablicy.
    if (pnum->size == pnum->count) {
        pnum->size *= 2;

        char **backup = pnum->numbers;
        size_t count = pnum->count;
        pnum->numbers = realloc(pnum->numbers, sizeof(char*) * pnum->size);

        if (pnum->numbers == NULL) {
            for (size_t i = 0; i < count; ++i) {
                free(backup[i]);
            }

            free(backup);
            return false;
        }
    }

    pnum->numbers[(pnum->count)++] = num;

    return true;
}

extern char const *phnumGet(PhoneNumbers const *pnum, size_t idx) {
    if (pnum == NULL || idx >= pnum->count) {
        return NULL;
    }
           
    return pnum->numbers[idx];
}

extern void phnumDelete(PhoneNumbers *pnum) {
    if (pnum != NULL) {
        for (size_t i = 0; i < pnum->count; ++i) {
            free(pnum->numbers[i]);
        }

        free(pnum->numbers);
        free(pnum);
    }
}

/**
 * @brief Zamiana danego napisu na taki, który da się porównać
 * za pomocą standardowego porównywania napisów.
 * 
 * Polega na zamianie niestandardowych cyfr ('*' oraz '#') na znaki kodu ASCII
 * występujące po '9'.
 * 
 * @param[in, out] string - napis do konwersji 
 * @return Wskaźnik na zmodyfikowany napis.
 */
static char *stringToCompare(char *const string) {
    size_t length = stringLength(string);
    for (size_t i = 0; i < length; ++i) {
        if (string[i] == '*') {
            string[i] = 10 + '0';
        }
        else if (string[i] == '#') {
            string[i] = 11 + '0';
        }
    }

    return string;
}

/**
 * @brief Zamiana napisu na oryginalny.
 * 
 * Polega na zamianie znaków odpowiadających '0' + 10 oraz '0' + 11
 * na oryginalne występujące tam znaki (*' oraz '#').
 * 
 * @param[in, out] string - napis do konwersji 
 * @return Wskaźnik na zmodyfikowany napis.
 */
static char *stringToOriginal(char *string) {
    for (size_t i = 0; string[i]; ++i) {
        if (string[i] == (10 + '0')) {
            string[i] = '*';
        }
        else if (string[i] == (11 + '0')) {
            string[i] = '#';
        }
    }

    return string;
}

/**
 * @brief Funkcja porównująca dwa napisy.
 * 
 * @param[in] str1 - pierwszy napis
 * @param[in] str2 - drugi napis
 * @return Zwraca wartość
 *         @p -1 jeśli pierwszy napis jest mniejszy od drugiego,
 *         @p 0 jeśli napisy są równe,
 *         @p 1 jeśli pierwszy napis jest większy od drugiego.
 */
static int sortString (const void *str1, const void *str2) {
    char *string1 = stringToCompare(*((char **const) str1));
    char *string2 = stringToCompare(*((char **const) str2));

    int result = strcmp(
        stringToCompare(string1),
        stringToCompare(string2)
    );

    string1 = stringToOriginal(string1);
    string2 = stringToOriginal(string2);

    return result;
}

extern void phnumSort(PhoneNumbers *pnum) {
    qsort(pnum->numbers, pnum->count, sizeof(char*), sortString);
}

extern PhoneNumbers *phnumRemoveDuplicates(PhoneNumbers *pnum) {
    if (pnum == NULL) {
        return NULL;
    }

    PhoneNumbers *phnumNoDuplicates = phnumNew();
    if (phnumNoDuplicates == NULL) {
        phnumDelete(pnum);
        return NULL;
    }

    for (size_t i = 0; i < pnum->count; ++i) {
        // Pierwszy napis do porównania - ostatni napis dodany
        // na wynikową strukturę (lub NULL jeśli jest to pierwsza iteracja).
        size_t phnumNDLen = phnumNoDuplicates->count;
        char *str1 = (phnumNDLen == 0 ?
                      NULL : phnumNoDuplicates->numbers[phnumNDLen - 1]);

        // Kolejny napis na danej strukturze.
        char *str2 = pnum->numbers[i];

        // Jeśli numer jest pierwszym dodawanym numerem lub napisy są różne,
        // dodajemy napis na wynikową strukturę.
        if (phnumNoDuplicates->count == 0 || strcmp(str1, str2) != 0) {
            char *copy = copyString(pnum->numbers[i]);
            if (!phnumAdd(phnumNoDuplicates, copy)) {
                free(copy);
                phnumDelete(phnumNoDuplicates);
                phnumDelete(pnum);
                
                return NULL;
            }
        }
    }

    phnumDelete(pnum);

    return phnumNoDuplicates;
}