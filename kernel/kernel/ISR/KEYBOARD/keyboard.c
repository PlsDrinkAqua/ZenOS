#include <kernel/tty.h>
#include <kernel/io.h>
#include <stdbool.h>

#define KEYBOARD_DATA_PORT 0x60

// Base (un-shifted) map
static const char scancode_to_ascii[128] = {
    0,  27, '1','2','3','4','5','6',    // 0x00–0x07
    '7','8','9','0','-','=','\b','\t',  // 0x08–0x0F
    'q','w','e','r','t','y','u','i',    // 0x10–0x17
    'o','p','[',']','\n',0,             // 0x18–0x1D
    'a','s','d','f','g','h','j','k',    // 0x1E–0x25
    'l',';','\'','`',0,'\\','z','x',    // 0x26–0x2D
    'c','v','b','n','m',',','.','/',    // 0x2E–0x35
    0,'*',0,' ',0,0,0,0,                // 0x36–0x3D
    // … rest initialize to 0 …
};

// Shift-modified map
static const char scancode_to_ascii_shift[128] = {
    0,  27, '!','@','#','$','%','^',    // 0x00–0x07
    '&','*','(',')','_','+','\b','\t',  // 0x08–0x0F
    'Q','W','E','R','T','Y','U','I',    // 0x10–0x17
    'O','P','{','}','\n',0,             // 0x18–0x1D
    'A','S','D','F','G','H','J','K',    // 0x1E–0x25
    'L',':','"','~',0,'|','Z','X',      // 0x26–0x2D
    'C','V','B','N','M','<','>','?',    // 0x2E–0x35
    0,'*',0,' ',0,0,0,0,                // 0x36–0x3D
    // … rest initialize to 0 …
};

// Track Shift state
static bool shift_down = false;

void keyboard_isr() {
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    // key-release? (bit7 set)
    if (scancode & 0x80) {
        if (scancode == 0xAA || scancode == 0xB6) {
            // left (0x2A+0x80) or right (0x36+0x80) Shift released
            shift_down = false;
        }
        return;
    }

    // key-press make codes
    if (scancode == 0x2A || scancode == 0x36) {
        // left or right Shift down
        shift_down = true;
        return;
    }

    // choose table based on Shift state
    char ascii = shift_down
        ? scancode_to_ascii_shift[scancode]
        : scancode_to_ascii[scancode];

    if (ascii) {
        terminal_putchar(ascii);
    }
}

