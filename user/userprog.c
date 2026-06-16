#include <stdio.h>
#include <unistd.h>

#define LINE_MAX 128

static int streq(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

static int starts_with(const char *s, const char *prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) {
            return 0;
        }
    }

    return 1;
}

static void write_str(const char *s) {
    const char *p = s;

    while (*p) {
        p++;
    }

    write(1, s, (unsigned)(p - s));
}

static void readline(char *buf, int max) {
    int len = 0;

    for (;;) {
        int c = getchar();

        if (c < 0) {
            continue;
        }

        if (c == '\n') {
            buf[len] = '\0';
            return;
        }

        if (c == '\b') {
            if (len > 0) {
                len--;
            }
            continue;
        }

        if (len + 1 < max) {
            buf[len++] = (char)c;
        }
    }
}

static void run_command(const char *line) {
    if (line[0] == '\0') {
        return;
    }

    if (streq(line, "help")) {
        puts("commands: help, echo, about");
        return;
    }

    if (streq(line, "about")) {
        puts("ZenOS userland shell");
        return;
    }

    if (starts_with(line, "echo ")) {
        puts(line + 5);
        return;
    }

    write_str("unknown command: ");
    puts(line);
}

void _start(void) {
    char line[LINE_MAX];

    puts("ZenOS shell");

    for (;;) {
        write_str("ZenOS> ");
        readline(line, sizeof(line));
        run_command(line);
    }

}
