#include <kernel/tty.h>
#include <kernel/io.h>

#define KEYBOARD_DATA_PORT 0x60

// A simple lookup table for non-shifted keys (set 1 scan codes) need to complete it in the future
static const char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', // 0x00 - 0x07
    '7', '8', '9', '0', '-', '=', '\b', '\t', // 0x08 - 0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', // 0x10 - 0x17
    'o', 'p', '[', ']', '\n', 0,   // 0x18 - 0x1D  (0 for Control)
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', // 0x1E - 0x25
    'l', ';', '\'', '`', 0, '\\', 'z', 'x', // 0x26 - 0x2D
    'c', 'v', 'b', 'n', 'm', ',', '.', '/', // 0x2E - 0x35
    0,   '*', 0,  ' ', 0, 0, 0, 0,  // 0x36 - 0x3D  (0 for keys like Alt, F keys, etc.)
    // the rest can be filled in as needed
};


void keyboard_isr() {
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    // Check for key press (ignore key release events)
    // In set 1, a key release is signaled by the high bit being set.
    if (scancode & 0x80) {
        // Key release: ignore for now
        return;
    }

    char ascii = scancode_to_ascii[scancode];
    
    // Now you can, for example, write the character to the terminal.
    terminal_putchar(ascii);
}

