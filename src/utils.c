/** @file utils.c 
 * Implementacja funkcji pomocniczych.
 * 
 * @author Jagoda Bobińska (jb438249@students.mimuw.edu.pl)
 * @copyright Uniwersytet Warszawski
 * @date 2022
 */

#include <stdlib.h>
#include <stdbool.h>

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

extern bool areStringsEqual(char const *num1, char const *num2) {
    if (!ifNumOk(num1) || !ifNumOk(num2)) {
        return false;
    }

    size_t len1 = stringLength(num1);
    size_t len2 = stringLength(num2);

    if (len1 != len2) {
        return false;
    }

    bool areEqual = true;
    for (size_t i = 0; i < len1; ++i) {
        areEqual &= (num1[i] == num2[i]);
    }

    return areEqual;
}