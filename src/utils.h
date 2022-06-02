/** @file utils.c 
 * Interfejs funkcji pomocniczych.
 * 
 * @author Jagoda Bobińska (jb438249@students.mimuw.edu.pl)
 * @copyright Uniwersytet Warszawski
 * @date 2022
 */

#ifndef __UTILS_H__
#define __UTILS_H__

/**
 * @brief Zamiana znaku na odpowiadającą mu liczbę.
 * 
 * @param[in] c - znak.
 * @return Odpowiadająca mu liczba.
 */
extern int toInt(char c);

/**
 * @brief Zmiana liczby na odpowiadający jej znak.
 * 
 * Funkcja odwrotna do toInt.
 * 
 * @param digit - liczba.
 * @return Odpowiadający jej znak.
 */
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

#endif /* __UTILS_H__ */