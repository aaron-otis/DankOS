#ifndef _PS2_H
#define _PS2_H

#include "../lib/stdlib.h"
#include "../lib/stdint.h"
#include "../lib/stdio.h"
#include <stddef.h>
#include "io.h"

extern int PS2_init(void);
extern int PS2_enable_device(int dev);
extern int PS2_disable_device(int dev);
extern int PS2_reset_device(int dev);
extern int PS2_write(uint8_t c);
extern uint8_t PS2_read();
extern uint8_t PS2_polling_read();
extern int PS2_write_data(uint8_t *data, size_t size);
extern int PS2_read_data(uint8_t **data, size_t size);
extern int PS2_send_command(uint8_t c);

#define PS2_DATA 0x60
#define PS2_COMMAND 0x64

#define PS2_READ_ZBYTE 0x20 
#define PS2_WRITE_ZBYTE 0x60 /* 'N' is the command byte & 0x1F. */
#define PS2_READ_NBYTE 0x21
#define PS2_WRITE_NBYTE 0x61 /* 'N' is the command byte & 0x1F. */

#define PS2_DISABLE_PORT_1 0xAD
#define PS2_ENABLE_PORT_1 0xAE
#define PS2_DISABLE_PORT_2 0xA7
#define PS2_ENABLE_PORT_2 0xA8

#define PS2_TEST_PORT_1 0xAB
#define PS2_TEST_PORT_2 0xA9
#define PS2_TEST_CONTROLLER 0xAA
#define PS2_DIAG_DUMP 0xAC

#define PS2_READ_CTL_INPUT 0xC0
#define PS2_READ_CTL_OUTPUT 0xD0
#define PS2_COPY_UPPER_TO_STATUS 0xC1
#define PS2_COPY_LOWER_TO_STATUS 0xC2
#define PS2_WRITE_CTL 0xD1 /* Write next byte to Controller Output Port . */

/* Write next byte to first PS/2 port output buffer. */
#define PS2_WRITE_PORT_1_OUTPUT 0xD2 
/* Write next byte to second PS/2 port output buffer. */
#define PS2_WRTIE_PORT_2_OUTPUT 0xD3
/* Write next byte to second PS/2 port input buffer. */
#define PS2_WRITE_PORT_2_INPUT
/* 
 * Pulse output line low for 6 ms. Bits 0 to 3 are used as a mask (0 = pulse 
 * line, 1 = don't pulse line) and correspond to 4 different output lines. 
 */
#define PS2_PULSE_OUTPUT 0xF0

/* Configuration byte flags. */
#define PS2_PORT_1_INTERRUPT 1
#define PS2_PORT_2_INTERRUPT (1 << 1)
#define PS2_SYS_FLAG (1 << 2)
#define PS2_PORT_1_CLOCK (1 << 4)
#define PS2_PORT_2_CLOCK (1 << 5)
#define PS2_PORT_TRANSLATION (1 << 6)

/* Output port flags. */
#define PS2_SYS_RESET 1 /* WARNING: Always set this flag. */
#define PS2_A_GATE (1 << 1)
#define PS2_PORT_2_CLOCK_OUTPUT (1 << 2)
#define PS2_PORT_2_DATA_OUTPUT (1 << 3)
#define PS2_OUT_BUF_FULL_PORT_1 (1 << 4)
#define PS2_OUT_BUF_FULL_PORT_2 (1 << 5)
#define PS2_PORT_1_CLOCK_OUTPUT (1 << 6)
#define PS2_PORT_1_DATA_OUTPUT (1 << 7)

/* Status register flags. */
#define PS2_STATUS_OUTPUT 1
#define PS2_STATUS_INPUT (1 << 1)
/* PS2_SYS_FLAG is also used here. */
#define PS2_CMD_OR_DATA (1 << 3) /* 0 for data for device, 1 for data for 
                                    command. */
#define PS2_UNKNOWN_1 (1 << 4)
#define PS2_UNKNOWN_2 (1 << 5)
#define PS2_TIME_OUT (1 << 6) /* 1 for a timeout error, 0 otherwise. */
#define PS2_PARITY_ERR (1 << 7) /* 1 for parity error, 0 otherwise. */

/* Test return values. */
#define PS2_SELF_TEST_PASS 0x55
#define PS2_SELF_TEST_FAIL 0xFC
#define PS2_PORT_TEST_PASS 0x00
#define PS2_PORT_CLOCK_LOW 0x01
#define PS2_PORT_CLOCK_HIGH 0x02
#define PS2_PORT_DATA_LOW 0x03
#define PS2_PORT_DATA_HIGH 0x04

#endif
