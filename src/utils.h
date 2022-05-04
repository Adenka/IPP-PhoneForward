#ifndef UTILS_H
#define UTILS_H

void wypiszBlad(size_t numerLinii);

void* safeMalloc(size_t rozm);

void* safeCalloc(size_t ile, size_t rozm);

void* safeRealloc(void *wsk, size_t rozm);

#endif /* UTILS_H */