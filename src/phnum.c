
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
            for (size_t i = 0; i<count; ++i) {
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
