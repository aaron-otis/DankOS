#include "kb_scancodes.h"
#include "keyboard.h"

const unsigned char KB_code_set_1[SCAN_CODE_SIZE] = { 0, KEY_ESC, '1', '2', '3', '4', 
    '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 
    'y', 'u', 'i', 'o', 'p', '[', ']', '\n', KEY_L_CTRL, 'a', 's', 'd', 'f', 
    'g', 'h', 'j', 'k', 'l', ';', '\'', '`', KEY_L_SHIFT, '\\', 'z', 'x', 'c', 
    'v', 'b',  'n', 'm', ',', '.', '/', KEY_R_SHIFT, '*', KEY_L_ALT, ' ', 
    KEY_CAPS_LOCK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_NUM_LOCK, KEY_SCROLL_LOCK, 
};
