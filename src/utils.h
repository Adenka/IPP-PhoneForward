/**
 * @file
 * Moduł z funkcjami pomocniczymi.
 * @author Jagoda Bobińska
 * @date 2022
 * 
 * @copyright Uniwersytet Watszawski
 * 
 */

#ifndef UTILS_H
#define UTILS_H

extern int toInt(char c);

extern char toChar(int digit);

/**
 * @brief Sprawdzenie czy numer jest poprawny.
 * 
 * @param[in] num - numer, którego poprawność mamy ocenić;
 * @return Wartość @p true jeśli numer poprawny,
 *         wartość @p false - numer jest NULLem,
 *         jest pustym napisem lub zawiera znaki inne niż cyfry
 */
bool ifNumOk(char const *num);

/**
 * @brief Zwaraca długość poprawnego napisu.
 * 
 * @param[in] string - napis, którego długość mamy określić.
 * @return Długość napisu.
 */
size_t stringLength(char const *string);

/**
 * @brief Utworzenie płytkiej kopii zadanego napisu.
 * 
 * @param[in] num - Zadany napis (numer).
 * @return Płytka kopia zadanego napisu (numeru).
 */
char *copyString(char const *num);

/**
 * @brief Zwraca maksimum z dwóch liczb. 
 * @return size_t 
 */
size_t max(size_t a, size_t b);

/**
 * @brief Zwraca minimum z dwóch liczb. 
 * @return size_t 
 */
size_t min(size_t a, size_t b);

#endif /* UTILS_H */