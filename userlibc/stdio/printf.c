#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

static int print_str(const char *s) {
    const char *p = s;

    if (!s) {
        s = "(null)";
        p = s;
    }

    while (*p) {
        p++;
    }

    size_t len = (size_t)(p - s);
    if (len == 0) {
        return 0;
    }

    if (write(1, s, len) < 0) {
        return EOF;
    }

    return (int)len;
}

static int print_char(char c) {
    if (putchar(c) == EOF) {
        return EOF;
    }

    return 1;
}

static int print_unsigned(unsigned int value, unsigned int base) {
    char buf[16];
    static const char digits[] = "0123456789abcdef";
    int i = 0;
    int written = 0;

    do {
        buf[i++] = digits[value % base];
        value /= base;
    } while (value);

    while (i > 0) {
        if (print_char(buf[--i]) == EOF) {
            return EOF;
        }
        written++;
    }

    return written;
}

static int print_signed(int value) {
    unsigned int magnitude;
    int written = 0;

    if (value < 0) {
        if (print_char('-') == EOF) {
            return EOF;
        }
        written++;
        magnitude = (unsigned int)(-(value + 1)) + 1;
    } else {
        magnitude = (unsigned int)value;
    }

    int n = print_unsigned(magnitude, 10);
    if (n == EOF) {
        return EOF;
    }

    return written + n;
}

int printf(const char *format, ...) {
    va_list args;
    int written = 0;

    va_start(args, format);

    while (*format) {
        int n;

        if (*format != '%') {
            n = print_char(*format++);
            if (n == EOF) {
                va_end(args);
                return EOF;
            }
            written += n;
            continue;
        }

        format++;

        switch (*format) {
            case '\0':
                n = print_char('%');
                format--;
                break;
            case '%':
                n = print_char('%');
                break;
            case 'c':
                n = print_char((char)va_arg(args, int));
                break;
            case 's':
                n = print_str(va_arg(args, const char *));
                break;
            case 'd':
                n = print_signed(va_arg(args, int));
                break;
            case 'u':
                n = print_unsigned(va_arg(args, unsigned int), 10);
                break;
            case 'x':
                n = print_unsigned(va_arg(args, unsigned int), 16);
                break;
            default:
                n = print_char('%');
                if (n != EOF) {
                    int m = print_char(*format);
                    n = (m == EOF) ? EOF : n + m;
                }
                break;
        }

        if (n == EOF) {
            va_end(args);
            return EOF;
        }

        written += n;

        if (*format) {
            format++;
        }
    }

    va_end(args);
    return written;
}
