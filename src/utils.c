#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

extern int toInt(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    }

    if (c == '*') {
        return 10;
    }

    if (c == '#') {
        return 11;
    }

    return -1;
}

extern char toChar(int digit) {
    if (0 <= digit && digit <= 9) {
        return digit + '0';
    }

    if (digit == 10) {
        return '*';
    }

    if (digit == 11) {
        return '#';
    }

    return 7;
}

extern bool ifNumOk(char const *num) {
    if (num == NULL || *num == '\0') {
        return false;
    }

    num = (char *)num;
    while (toInt(*num) >= 0) {
        num += sizeof(char);
    }

    if (*num == '\0') {
        return true;
    }

    return false;
}

extern size_t stringLength(char const *string) {
    size_t length = 0;

    while (toInt(*string) >= 0) {
        string += sizeof(char);
        length++;
    }

    return length;
}

extern char *copyString(char const *num) {
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

extern size_t max(size_t a, size_t b) {
    if (a <= b) {
        return b;
    }

    return a;
}

extern size_t min(size_t a, size_t b) {
    if (a > b) {
        return b;
    }

    return a;
}