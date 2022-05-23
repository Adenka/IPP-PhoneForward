#include <string.h>
#include <stdio.h>
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

extern void phnumPrint(PhoneNumbers *pnum) {
    printf("pnum:\n");
    for (size_t i = 0; i < pnum->count; ++i) {
        for (size_t j = 0; j < stringLength(pnum->numbers[i]); ++j) {
            printf("%c", pnum->numbers[i][j]);
        }
        printf("\n");
    }
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

static char *stringToCompare(char *const string) {
    for (size_t i = 0; i < stringLength(string); ++i) {
        if (string[i] == '*') {
            string[i] = 10 + '0';
        }
        else if (string[i] == '#') {
            string[i] = 11 + '0';
        }
    }

    return string;
}

static int sortString (const void *str1, const void *str2) {
    char *const *string1 = str1;
    char *const *string2 = str2;

    return strcmp(
        stringToCompare(copyString(*string1)),
        stringToCompare(copyString(*string2))
    );
}

extern void phnumSort(PhoneNumbers *pnum) {
    qsort(pnum->numbers, pnum->count, sizeof(char*), sortString);
}

//TODO - do jednego warunku - ulepszyć ifa
extern PhoneNumbers *phnumRemoveDuplicates(PhoneNumbers *pnum) {
    PhoneNumbers *phnumNoDuplicates = phnumNew();
    for (size_t i = 0; i < pnum->count; ++i) {
        if (phnumNoDuplicates->count == 0) {
            phnumAdd(phnumNoDuplicates, copyString(pnum->numbers[i]));
        }
        else {
            if (strcmp(phnumNoDuplicates->numbers[phnumNoDuplicates->count - 1], pnum->numbers[i]) != 0) {
                phnumAdd(phnumNoDuplicates, copyString(pnum->numbers[i]));
            }
        }
    }

    phnumDelete(pnum);
    return phnumNoDuplicates;
}