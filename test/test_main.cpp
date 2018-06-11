/*
 * test_main.cpp
 *
 *  Created on: 08.06.2018
 *      Author: tixiv
 */

#include <string.h>
#include "UnitTest++/UnitTest++.h"

#include "../../slcan/slcan.h"

SUITE(SlcanTest)
{
  class SLCanFixture
  {
  public:
    slcan_can_mode_e mode;
    uint8_t baud;
    static bool can_init_mock(slcan_can_mode_e mode, uint8_t baud){
      _this->mode = mode;
      _this->baud = baud;
      return true;
    }

    char uart_putstr_string[100];
    static void uart_putstr_mock(const char * str){
      strcat(_this->uart_putstr_string, str);
    }

    can_message_t can_buffer;
    int can_messages_in_use;
    static can_message_t * can_buffer_get_mock(void){
      _this->can_messages_in_use++;
      return &_this->can_buffer;
    }

    static bool can_transmit_mock(const can_message_t * cmsg){
      if(cmsg == &_this->can_buffer)
        _this->can_messages_in_use--;
      else
        throw 1;
      if(cmsg->dlc > 8) throw 2;
      if(cmsg->IDE == 0){
        if(cmsg->id > 0x7ff) throw 3;
      }
      else
      {
        if(cmsg->id > 0x1fffffff) throw 4;
      }
      return true;
    }

    uint8_t status;
    static uint8_t can_get_status_mock(){
      return _this->status;
    }

    static SLCanFixture * _this;

    slcan_t mySlcan;

    SLCanFixture(){
      _this = this;
      can_messages_in_use = 0;
      status = 0;
      baud = -1;
      mode = can_mode_close;
      memset(uart_putstr_string, 0, 100);
      memset(&mySlcan, 0, sizeof(mySlcan));
      memset(&can_buffer, 0, sizeof(can_buffer));

      mySlcan.can_init_callback       = can_init_mock;
      mySlcan.uart_putstr_callback    = uart_putstr_mock;
      mySlcan.can_buffer_get_callback = can_buffer_get_mock;
      mySlcan.can_transmit_callback   = can_transmit_mock;
      mySlcan.can_get_status_callback = can_get_status_mock;
    }
  };

  SLCanFixture * SLCanFixture::_this;

  TEST_FIXTURE(SLCanFixture, OpenNormal){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    CHECK(strcmp("\r\r", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
  }

  TEST_FIXTURE(SLCanFixture, OpenListen){
    slcan_execute_command(&mySlcan, "S5");
    slcan_execute_command(&mySlcan, "L");
    CHECK(strcmp("\r\r", uart_putstr_string) == 0);
    CHECK_EQUAL(5, baud);
    CHECK_EQUAL(can_mode_listen, mode);
  }

  TEST_FIXTURE(SLCanFixture, OpenLoopBack){
    slcan_execute_command(&mySlcan, "S6");
    slcan_execute_command(&mySlcan, "l");
    CHECK(strcmp("\r\r", uart_putstr_string) == 0);
    CHECK_EQUAL(6, baud);
    CHECK_EQUAL(can_mode_loopback, mode);
  }

  TEST_FIXTURE(SLCanFixture, OpenClose){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "C");
    CHECK(strcmp("\r\r\r", uart_putstr_string) == 0);
    CHECK_EQUAL(can_mode_close, mode);
  }

  TEST_FIXTURE(SLCanFixture, OpenWithoutBaud){
    slcan_execute_command(&mySlcan, "O");
    CHECK(strcmp("\a", uart_putstr_string) == 0);
    CHECK_EQUAL(can_mode_close, mode);
  }

  TEST_FIXTURE(SLCanFixture, ChangeBaudWhileOpen){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "S5");
    CHECK(strcmp("\r\r\a", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
  }

  TEST_FIXTURE(SLCanFixture, TransmitStd1){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "t12381122334455667788");
    CHECK(strcmp("\r\rz\r", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x123, can_buffer.id);
    CHECK_EQUAL(0, can_buffer.IDE);
    CHECK_EQUAL(0, can_buffer.RTR);
    CHECK_EQUAL(8, can_buffer.dlc);
    uint8_t expected_data[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    CHECK_ARRAY_EQUAL(expected_data, can_buffer.data, 8);
  }

  TEST_FIXTURE(SLCanFixture, Transmitloopback){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "l");
    slcan_execute_command(&mySlcan, "t12381122334455667788");
    CHECK(strcmp("\r\rz\r", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_loopback, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x123, can_buffer.id);
    CHECK_EQUAL(0, can_buffer.IDE);
    CHECK_EQUAL(0, can_buffer.RTR);
    CHECK_EQUAL(8, can_buffer.dlc);
    uint8_t expected_data[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    CHECK_ARRAY_EQUAL(expected_data, can_buffer.data, 8);
  }

  TEST_FIXTURE(SLCanFixture, TransmitStd2){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "t7BC6AABBCCDDEEFF");
    CHECK(strcmp("\r\rz\r", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x7BC, can_buffer.id);
    CHECK_EQUAL(0, can_buffer.IDE);
    CHECK_EQUAL(0, can_buffer.RTR);
    CHECK_EQUAL(6, can_buffer.dlc);
    uint8_t expected_data[8] = {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
    CHECK_ARRAY_EQUAL(expected_data, can_buffer.data, 6);
  }

  TEST_FIXTURE(SLCanFixture, TransmitStdEmpty){
    slcan_execute_command(&mySlcan, "S0");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "t4560");
    CHECK(strcmp("\r\rz\r", uart_putstr_string) == 0);
    CHECK_EQUAL(0, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x456, can_buffer.id);
    CHECK_EQUAL(0, can_buffer.IDE);
    CHECK_EQUAL(0, can_buffer.RTR);
    CHECK_EQUAL(0, can_buffer.dlc);
  }

  TEST_FIXTURE(SLCanFixture, TransmitExt){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "T12ABCDEF81234567812345678");
    CHECK(strcmp("\r\rZ\r", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x12ABCDEF, can_buffer.id);
    CHECK_EQUAL(1, can_buffer.IDE);
    CHECK_EQUAL(0, can_buffer.RTR);
    CHECK_EQUAL(8, can_buffer.dlc);
    uint8_t expected_data[8] = {0x12,0x34,0x56,0x78,0x12,0x34,0x56,0x78};
    CHECK_ARRAY_EQUAL(expected_data, can_buffer.data, 8);
  }

  TEST_FIXTURE(SLCanFixture, TransmitExtEmpty){
    slcan_execute_command(&mySlcan, "S8");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "T123412340");
    CHECK(strcmp("\r\rZ\r", uart_putstr_string) == 0);
    CHECK_EQUAL(8, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x12341234, can_buffer.id);
    CHECK_EQUAL(1, can_buffer.IDE);
    CHECK_EQUAL(0, can_buffer.RTR);
    CHECK_EQUAL(0, can_buffer.dlc);
  }

  TEST_FIXTURE(SLCanFixture, TransmitStdRtr){
    uint8_t expected_data[8] = {0x12,0x34,0x56,0x78,0x12,0x34,0x56,0x78};
    memcpy(can_buffer.data, expected_data, 8);
    slcan_execute_command(&mySlcan, "S8");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "r6788");
    CHECK(strcmp("\r\rz\r", uart_putstr_string) == 0);
    CHECK_EQUAL(8, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x678, can_buffer.id);
    CHECK_EQUAL(0, can_buffer.IDE);
    CHECK_EQUAL(1, can_buffer.RTR);
    CHECK_EQUAL(8, can_buffer.dlc);
    CHECK_ARRAY_EQUAL(expected_data, can_buffer.data, 8);
  }

  TEST_FIXTURE(SLCanFixture, TransmitExtRtr){
    uint8_t expected_data[8] = {0x12,0x34,0x56,0x78,0x12,0x34,0x56,0x78};
    memcpy(can_buffer.data, expected_data, 8);
    slcan_execute_command(&mySlcan, "S8");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "R188888888");
    CHECK(strcmp("\r\rZ\r", uart_putstr_string) == 0);
    CHECK_EQUAL(8, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0x18888888, can_buffer.id);
    CHECK_EQUAL(1, can_buffer.IDE);
    CHECK_EQUAL(1, can_buffer.RTR);
    CHECK_EQUAL(8, can_buffer.dlc);
    CHECK_ARRAY_EQUAL(expected_data, can_buffer.data, 8);
  }

  TEST_FIXTURE(SLCanFixture, TransmitStdBadDlc){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "t12391122334455667788");
    CHECK(strcmp("\r\r\a", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
  }

  TEST_FIXTURE(SLCanFixture, TransmitStdBadId){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "t80081122334455667788");
    CHECK(strcmp("\r\r\a", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
  }

  TEST_FIXTURE(SLCanFixture, TransmitExtBadId){
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "T8888888881122334455667788");
    CHECK(strcmp("\r\r\a", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_normal, mode);
    CHECK_EQUAL(0,can_messages_in_use);
  }

  TEST_FIXTURE(SLCanFixture, TransmitWhileClosed){
    can_buffer.id = 0xfff;
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "C");
    slcan_execute_command(&mySlcan, "t12381122334455667788");
    CHECK(strcmp("\r\r\r\a", uart_putstr_string) == 0);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0xfff, can_buffer.id);
  }

  TEST_FIXTURE(SLCanFixture, TransmitWhileListen){
    can_buffer.id = 0xfff;
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "L");
    slcan_execute_command(&mySlcan, "t12381122334455667788");
    CHECK(strcmp("\r\r\a", uart_putstr_string) == 0);
    CHECK_EQUAL(4, baud);
    CHECK_EQUAL(can_mode_listen, mode);
    CHECK_EQUAL(0,can_messages_in_use);
    CHECK_EQUAL(0xfff, can_buffer.id);
  }


  TEST_FIXTURE(SLCanFixture, ReceiveStd){
    uint8_t data[8] = {0x12,0x34,0x56,0x78,0x12,0x34,0x56,0x78};
    can_message_t cmsg;
    cmsg.IDE = 0;
    cmsg.RTR = 0;
    cmsg.dlc = 8;
    cmsg.id = 0x1A9;
    memcpy(cmsg.data, data, 8);
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_handle_can_message(&mySlcan, &cmsg);
    CHECK(strcmp("\r\rt1A981234567812345678\r", uart_putstr_string) == 0);
  }

  TEST_FIXTURE(SLCanFixture, ReceiveExt){
    uint8_t data[8] = {0xAB,0xCD,0xEF,0x12,0x34,0x56,0x78,0x9A};
    can_message_t cmsg;
    cmsg.IDE = 1;
    cmsg.RTR = 0;
    cmsg.dlc = 8;
    cmsg.id = 0x1234ABCD;
    memcpy(cmsg.data, data, 8);
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_handle_can_message(&mySlcan, &cmsg);
    CHECK(strcmp("\r\rT1234ABCD8ABCDEF123456789A\r", uart_putstr_string) == 0);
  }

  TEST_FIXTURE(SLCanFixture, ReceiveStdRtr){
    can_message_t cmsg;
    cmsg.IDE = 0;
    cmsg.RTR = 1;
    cmsg.dlc = 4;
    cmsg.id = 0x789;
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_handle_can_message(&mySlcan, &cmsg);
    CHECK(strcmp("\r\rr7894\r", uart_putstr_string) == 0);
  }

  TEST_FIXTURE(SLCanFixture, ReceiveExtRtr){
    can_message_t cmsg;
    cmsg.IDE = 1;
    cmsg.RTR = 1;
    cmsg.dlc = 4;
    cmsg.id = 0x18765432;
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_handle_can_message(&mySlcan, &cmsg);
    CHECK(strcmp("\r\rR187654324\r", uart_putstr_string) == 0);
  }

  TEST_FIXTURE(SLCanFixture, ReceiveWhileClosed){
    uint8_t data[8] = {0x12,0x34,0x56,0x78,0x12,0x34,0x56,0x78};
    can_message_t cmsg;
    cmsg.IDE = 0;
    cmsg.RTR = 0;
    cmsg.dlc = 8;
    cmsg.id = 0x1A9;
    memcpy(cmsg.data, data, 8);
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "C");
    slcan_handle_can_message(&mySlcan, &cmsg);
    CHECK(strcmp("\r\r\r", uart_putstr_string) == 0);
  }

  TEST_FIXTURE(SLCanFixture, Status1){
    status = 0x45;
    slcan_execute_command(&mySlcan, "S4");
    slcan_execute_command(&mySlcan, "O");
    slcan_execute_command(&mySlcan, "F");
    CHECK(strcmp("\r\rF45\r", uart_putstr_string) == 0);
  }

  TEST_FIXTURE(SLCanFixture, Status2){
    status = 0xAF;
    slcan_execute_command(&mySlcan, "F");
    CHECK(strcmp("FAF\r", uart_putstr_string) == 0);
  }

  TEST_FIXTURE(SLCanFixture, Version){
    slcan_execute_command(&mySlcan, "V");
    CHECK('V' == uart_putstr_string[0]);
    for(int i=1; i<5; i++)
      CHECK('0' <= uart_putstr_string[i] && '9' >= uart_putstr_string[i]);
    CHECK('\r' == uart_putstr_string[5]);
    CHECK(0 == uart_putstr_string[6]);
  }

  TEST_FIXTURE(SLCanFixture, Serial){
    slcan_execute_command(&mySlcan, "N");
    CHECK('N' == uart_putstr_string[0]);
    for(int i=1; i<5; i++)
      CHECK('0' <= uart_putstr_string[i] && '9' >= uart_putstr_string[i]);
    CHECK('\r' == uart_putstr_string[5]);
    CHECK(0 == uart_putstr_string[6]);
  }


}



int main(int, const char *[])
{
   return UnitTest::RunAllTests();
}
