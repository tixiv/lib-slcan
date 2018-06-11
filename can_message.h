/*
 * can_message.h
 *
 *  Created on: 01.06.2018
 *      Author: tixiv
 */

#ifndef CANLIB_CAN_MESSAGE_H_
#define CANLIB_CAN_MESSAGE_H_

typedef struct
{
  struct {
    uint8_t dlc:4;
    uint8_t RTR:1;
    uint8_t IDE:1;
  };
  uint32_t id;
  uint8_t data[8];
} can_message_t;

#endif /* CANLIB_CAN_MESSAGE_H_ */
