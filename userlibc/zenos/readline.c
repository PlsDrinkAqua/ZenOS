#include <stdio.h>
#include <unistd.h>
#include <zenos/readline.h>

char *zenos_readline(char *buf, size_t size) {
    size_t len = 0;

    if (size == 0) {
        return buf;
    }

    for (;;) {
        char c;

        if (read(0, &c, 1) != 1) {
            continue;
        }

        if (c == '\n') {
            putchar('\n');
            buf[len] = '\0';
            return buf;
        }

        if (c == '\b') {
            if (len > 0) {
                len--;
                putchar('\b');
            }
            continue;
        }

        if (len + 1 < size) {
            buf[len++] = c;
            putchar(c);
        }
    }
}
