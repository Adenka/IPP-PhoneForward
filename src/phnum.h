/**
 * @file
 * Moduł zawierający funkcje związane ze strukturą PhoneNumbers.
 * @author Jagoda Bobińska <jb438249@students.mimuw.edu.pl>
 * @date 2022
 * 
 * @copyright Uniwersytet Warszawski
 * 
 */
#ifndef PHNUM_H
#define PHNUM_H

/**
 * Struktura przechowująca ciąg numerów telefonów.
 */
struct PhoneNumbers;

/**
 * @brief Utworzenie nowej struktury przechowywującej ciąg numerów telefonów.
 * 
 * @return Wskaźnik na nową strukturę lub NULL jeśli alokacja pamięci się nie powiodła.
 */
PhoneNumbers *phnumNew(void);

/**
 * @brief Dodanie nowego numeru na strukturę przechowującą ciąg numerów telefonów.
 * 
 * @param[in, out] phnum - struktura przechowująca ciąg numerów telefonów;
 * @param[in] num - numer, który mamy dodać na strukturę.
 * @return Wartość @p true jeśli dodanie się powiodło,
 *          @p false w przeciwnym wypadku.
 */
bool phnumAdd(PhoneNumbers *phnum, char *num);

/** @brief Udostępnia numer.
 * Udostępnia wskaźnik na napis reprezentujący numer. Napisy są indeksowane
 * kolejno od zera.
 * @param[in] pnum – wskaźnik na strukturę przechowującą ciąg numerów telefonów;
 * @param[in] idx  – indeks numeru telefonu.
 * @return Wskaźnik na napis reprezentujący numer telefonu. Wartość NULL, jeśli
 *         wskaźnik @p pnum ma wartość NULL lub indeks ma za dużą wartość.
 */
char const *phnumGet(PhoneNumbers const *pnum, size_t idx);

/** @brief Usuwa1 strukturę.
 * Usuwa strukturę wskazywaną przez @p pnum. Nic nie robi, jeśli wskaźnik ten ma
 * wartość NULL.
 * @param[in] pnum – wskaźnik na usuwaną strukturę.
 */
void phnumDelete(PhoneNumbers *pnum);

extern void phnumSort(PhoneNumbers *pnum);

extern PhoneNumbers *phnumRemoveDuplicates(PhoneNumbers *pnum);

extern void phnumPrint(PhoneNumbers *pnum);

#endif /* PHNUM_H */