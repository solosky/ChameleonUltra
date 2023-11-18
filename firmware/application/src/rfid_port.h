#ifndef RFID_PORT_H
#define RFID_PORT_H

#include "bsp.h"

#define ALIGN_U32  __attribute__((aligned(4)))

#define TAG_FIELD_LED_OFF()  bsp_board_led_off(BSP_BOARD_LED_0);
#define TAG_FIELD_LED_ON()  bsp_board_led_on(BSP_BOARD_LED_0);

#define set_slot_light_color(a)
#define sleep_timer_start(a)
#define sleep_timer_stop(a)


#endif 