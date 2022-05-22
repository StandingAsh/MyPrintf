//
// Created by standingash on 21. 12. 18..
//

#ifndef MYPRINTF_MY_PRINTF_H
#define MYPRINTF_MY_PRINTF_H

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define FORMAT "csdpxX"

#define STR1 1
#define STR2 2
#define BOTH 0

typedef struct {
    char format;

    int isMinus;
    int isZero;

    int width;
    int precision;

    int base;
    char sign;
} Flags;

// my printf
int my_printf(const char *string, ...);

void setFlags(va_list ap, char *string, Flags *flags, int index);
void setWidthAndPrecision(va_list ap, char *format, Flags *flags, int index);

int printByFormat(va_list ap, Flags *flags);
void initialize(Flags *info);
char *numberString(char type) ;

int printChar(int c, Flags *flags);
int printString(char *string, Flags *flags);
int printInteger(unsigned long long number, Flags *flags);

char *makeBuffer(char *string, int end, int length);

int putWidth(int width, int stringLength, int zero);
int addWidth(char **buffer, Flags *flags);
int addPrecision(unsigned long long number, Flags *flags, char **buffer);

int addMinus(Flags *flags, char **buffer);
int addMinus2(int bufferLength, Flags *flags, char **buffer);
int addPrefix(char **buffer);

#endif //MYPRINTF_MY_PRINTF_H
