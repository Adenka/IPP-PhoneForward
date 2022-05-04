#include <stdlib.h>
#include <stdio.h>

extern void wypiszBlad(size_t numerLinii) {
    fprintf(stderr, "ERROR %lu\n", numerLinii);
    exit(1);
}

/* Funkcje alokujące pamięć analogicznie do standardowych,
ale zwracają błąd przy nieudanej alokacji pamięci */

//TODO - zmienić na ang
extern void *safeMalloc(size_t rozm) {
    void *wsk = malloc(rozm);

    if (!wsk) {
        wypiszBlad(0);
    }
        
    return wsk;
}

extern void *safeCalloc(size_t ile, size_t rozm) {
    void *wsk = calloc(ile, rozm);

    if (!wsk) {
        wypiszBlad(0);
    }
        
    return wsk;
}

extern void *safeRealloc(void *wsk, size_t rozm) {
    wsk = realloc(wsk, rozm);

    if (!wsk) {
        wypiszBlad(0);
    }
        
    return wsk;
}