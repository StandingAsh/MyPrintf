#include "my_printf.h"

int my_printf(const char *string, ...) {
    int length;
    va_list ap;

    va_start(ap, string);
    Flags *flags;

    if (!(flags = (Flags *) malloc(sizeof(Flags) * 1)))
        return -1;

    for (int index = 0; string[index] != '\0';) {
        while (string[index] != '%' && string[index] != '\0')
            length += write(STDOUT_FILENO, &string[index++], 1);

        if (string[index] == '%') {
            initialize(flags);
            while (string[++index] != '\0' && !strchr(FORMAT, string[index]))
                setFlags(ap, string, flags, index);

            flags->format = string[index++];
            if ((flags->isMinus || flags->precision > -1) && flags->format != '%')
                flags->isZero = 0;

            length += printByFormat(ap, flags);
        }
    }

    free(flags);

    va_end(ap);
    return length;
}

void setFlags(va_list ap, char *string, Flags *flags, int index) {
    if (string[index] == '0' && flags->width == 0 && flags->precision == -1)
        flags->isZero = 1;
    else if (string[index] == '-')
        flags->isMinus = 1;
    else if (string[index] == '.')
        flags->precision = 0;
    else if (isdigit(string[index]) || string[index] == '*')
        setWidthAndPrecision(ap, string, flags, index);
}

void setWidthAndPrecision(va_list ap, char *format, Flags *flags, int index) {
    if (isdigit(format[index])) {
        if (flags->precision == -1)
            flags->width = flags->width * 10 + format[index] - 48;
        else
            flags->precision = flags->precision * 10 + format[index] - 48;
    } else if (format[index] == '*') {
        if (flags->precision == -1) {
            flags->width = va_arg(ap, int);
            if (flags->width < 0) {
                flags->isMinus = 1;
                flags->width *= -1;
            }
        } else
            flags->precision = va_arg(ap, int);
    }
}

int printByFormat(va_list ap, Flags *flags) {
    int length = 0;
    char format;

    format = flags->format;

    switch (format) {
        case 'c':
            length = printChar(va_arg(ap, int), flags);
            break;
        case 's':
            length = printString(va_arg(ap, char*), flags);
            break;
        case 'd':
            length = printInteger(va_arg(ap, int), flags);
            break;
        case 'x':
        case 'X':
            length = printInteger(va_arg(ap, unsigned int), flags);
            break;
        case 'p':
            length = printInteger(va_arg(ap, unsigned long long), flags);
    }

    return length;
}

char *addEnd(char *string1, char *string2, int flag) {
    char *result;
    int index = 0;

    result = (char *) malloc(sizeof(char) * (strlen(string1) + strlen(string2) + 1));
    if (!result)
        return 0;

    for (index = 0; index < strlen(string1); index++)
        result[index] = string1[index];


    if (flag == STR1 || flag == BOTH)
        free(string1);

    for (int i = 0; i < strlen(string2); i++)
        result[index++] = string2[i];
    result[index] = '\0';

    if (flag == STR2 || flag == BOTH)
        free(string2);

    return result;
}

int printChar(int c, Flags *flags) {
    int length = 0;

    if (flags->isMinus)
        length += write(STDOUT_FILENO, &c, 1);
    length += putWidth(flags->width, 1, flags->isZero);

    if (!flags->isMinus)
        length += write(STDOUT_FILENO, &c, 1);

    return length;
}

int putWidth(int width, int stringLength, int zero) {
    int length = 0;
    char fill;

    if (zero == 1)
        fill = '0';
    else
        fill = ' ';

    for (int i = stringLength; i < width; i++) {
        write(STDOUT_FILENO, &fill, 1);
        length++;
    }

    return length;
}

int printString(char *string, Flags *flags) {
    int length = 0;
    char *buffer;

    if (string == NULL)
        string = "(null)";

    if (flags->precision == -1 || flags->precision > strlen(string))
        flags->precision = strlen(string);

    buffer = makeBuffer(string, flags->precision, strlen(string));
    length = addWidth(&buffer, flags);

    for (int i = 0; buffer[i] != '\0'; i++)
        write(STDOUT_FILENO, &buffer[i], 1);

    free(buffer);
    return length;
}

char *makeBuffer(char *string, int end, int length) {
    char *buffer;
    int index = 0;

    if (end > length)
        end = length;

    buffer = (char *) malloc(sizeof(char) * end + 1);
    if (!buffer)
        return NULL;

    for (index = 0; index < end; index++)
        buffer[index] = string[index];
    buffer[index] = '\0';

    return buffer;
}

int addWidth(char **buffer, Flags *flags) {
    char *width;
    char fill;

    int index = 0;

    if (flags->width <= strlen(*buffer))
        return strlen(*buffer);
    width = (char *) malloc(sizeof(char) * (flags->width - strlen(*buffer) + 1));

    if (flags->isZero == 1)
        fill = '0';
    else
        fill = ' ';

    for (index = 0; index < flags->width - strlen(*buffer); index++)
        width[index] = fill;
    width[index] = '\0';

    if (flags->isMinus == 0)
        *buffer = addEnd(width, *buffer, BOTH);
    else
        *buffer = addEnd(*buffer, width, BOTH);

    return flags->width;
}

int printInteger(unsigned long long number, Flags *flags) {
    char *buffer;
    int length = 0, result = 0;

    if (flags->format == 'x' || flags->format == 'X' || flags->format == 'p')
        flags->base = 16;

    if ((flags->format == 'd') && (long long) number < 0) {
        flags->sign = '-';
        number *= -1;
    }

    length = addPrecision(number, flags, &buffer);
    length += addMinus(flags, &buffer);

    if (flags->format == 'p')
        length = addPrefix(&buffer);
    result = addWidth(&buffer, flags);
    result += addMinus2(length, flags, &buffer);

    for (int i = 0; i < strlen(buffer); i++)
        write(STDOUT_FILENO, &buffer[i], 1);

    free(buffer);
    return result;
}

int addPrecision(unsigned long long number, Flags *flags, char **buffer) {
    int bufferLength = 1, length;
    unsigned long long temp = number;

    if (number != 0 || flags->precision == 0) {
        bufferLength = 0;

        while (temp > 0) {
            temp /= flags->base;
            bufferLength++;
        }
    }

    length = (flags->precision > bufferLength) ? flags->precision : bufferLength;
    if (!(*buffer = (char *) malloc(sizeof(char) * length + 1)))
        return 0;
    (*buffer)[length] = '\0';

    for (int i = 0; bufferLength + i < length; i++)
        (*buffer)[i] = '0';

    if (number == 0 && flags->precision != 0)
        (*buffer)[length - 1] = '0';

    for (int i = 1; number > 0; i++, number /= flags->base)
        (*buffer)[length - i] = numberString(flags->format)[number % flags->base];

    return length;
}

int addMinus(Flags *flags, char **buffer) {
    int length = 0;

    if ((flags->format == 'd')
        && !flags->isZero && flags->sign == '-') {
        *buffer = addEnd("-", *buffer, STR2);
        length = 1;
    }

    return length;
}

int addMinus2(int bufferLength, Flags *flags, char **buffer) {
    int length = 0;

    if (flags->sign == '-' && flags->isZero) {
        if (bufferLength >= flags->width) {
            *buffer = addEnd("-", *buffer, STR2);
            length = 1;
        } else if (bufferLength < flags->width)
            *buffer[0] = '-';
    }

    return length;
}

int addPrefix(char **buffer) {
    *buffer = addEnd("0x", *buffer, STR2);

    return strlen(*buffer);
}

void initialize(Flags *info) {
    info->isMinus = 0;
    info->isZero = 0;
    info->width = 0;
    info->precision = -1;
    info->format = 0;
    info->base = 10;
    info->sign = '+';
}

char *numberString(char type) {
    switch (type) {
        case 'd':
            return "0123456789";
            break;
        case 'x':
        case 'p':
            return "0123456789abcdef";
            break;
        case 'X':
            return "0123456789ABCDEF";
            break;
        default:
            return 0;
            break;
    }
}
