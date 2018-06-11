/*
 * slcan.h
 *
 *  Created on: 01.06.2018
 *      Author: tixiv
 */

#ifndef SLCAN_H_
#define SLCAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "can_message.h"

typedef enum{
  can_mode_close,
  can_mode_normal,
  can_mode_listen,
  can_mode_loopback,
}slcan_can_mode_e;

typedef bool can_init_func_t(slcan_can_mode_e mode, uint8_t baud);
typedef bool can_transmit_func_t(const can_message_t *);
typedef void uart_putstr_func_t(const char *);
typedef can_message_t *can_buffer_get_func_t(void);
typedef uint8_t can_get_status_func_t(void);

typedef struct{
  can_init_func_t * can_init_callback;
  uart_putstr_func_t * uart_putstr_callback;
  can_buffer_get_func_t * can_buffer_get_callback;
  can_transmit_func_t * can_transmit_callback;
  can_get_status_func_t * can_get_status_callback;

  uint8_t terminal_open:1;
  uint8_t transmit_enabled:1;
  uint8_t baudrate_configured:1;
  uint8_t baud;
}slcan_t;

// for microcontroller with slcan_t callback struct
void slcan_execute_command(slcan_t * lp, const char * str);
void slcan_handle_can_message(slcan_t * lp, can_message_t *cmsg);

// for using on PC
bool slcan_can_message_from_string(can_message_t * cmsg, const char * str);
void slcan_string_from_can_message(char * str, can_message_t *cmsg);
const char * slcan_get_baud_string(uint32_t bps);

#ifdef __cplusplus
}
#endif

#endif /* SLCAN_H_ */
