#ifndef _KEYBOARD_H
#define _KEYBOARD_H

/* Commands. */
#define KB_SET_LEDS 0xED
#define KB_ECHO 0xEE

#define KB_GET_SET_SCAN_CODE 0xF0
#define KB_GET_SCAN_CODE 0x0
#define KB_SCAN_CODE_1 0x1
#define KB_SCAN_CODE_2 0x2
#define KB_SCAN_CODE_3 0x3
#define KB_ID_KEYBOARD 0xF2 /* 0xFA (ACK) followed by none or more ID bytes. */

/* 
 * Use bits 0 to 4. Repeat rate (00000b = 30 Hz, ..., 11111b = 2 Hz).
 * Use bits 5 to 6. Delay before keys repeat (00b = 250 ms, 01b = 500 ms, 
 * 10b = 750 ms, 11b = 1000 ms).
 * Bit 7 must be 0.
 */
#define KB_SET_RATE_DELAY 0xF3 /* 0xFA (ACK) or 0xFE (Resend) . */
#define KB_ENABLE_SCANNING 0xF4 /* 0xFA (ACK) or 0xFE (Resend). */
#define KB_DISABLE_SCANNING 0xF5 /* 0xFA (ACK) or 0xFE (Resend). */
#define KB_SET_DEFAULT_PARAMA 0xF6
#define KB_ALL_TYPEMATIC 0xF7
#define KB_ALL_MAKE_RELEASE 0xF8
#define KB_ALL_MAKE 0xF9
#define KB_ALL_ALL_TYPES 0xFA
#define KB_SET_TYPEMATIC 0xFB /* Needs key scan code afterwards. */
#define KB_SET_MAKE_RELEASE 0xFC /* Needs key scan code afterwards. */
#define KB_SET_MAKE 0xFD /* Needs key scan code afterwards. */
#define KB_RESEND_LAST_BYTE 0xFE
#define KB_RESET_TEST 0xFF /* Reset and run self test. */

/* Responses. */
#define KB_KEY_DETECT_ERROR 0x00
#define KB_PASSED_SELF_TEST 0xAA
#define KB_ECHO_RESPONSE 0xEE
#define KB_CMD_ACK 0xFA
#define KB_FAILED_SELF_TEST 0xFC
#define KB_FAILED_POWER_UP 0xFD
#define KB_RESEND_CMD 0xFE
#define KB_KEY_DETECT_ERROR_2 0xFF

/* Scan codes. */

#endif
