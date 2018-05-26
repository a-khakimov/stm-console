#ifndef __CONSOLE_H
#define __CONSOLE_H


#include "stm32f7xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

void 	set_receive_finish( const uint8_t state);
uint8_t get_receive_finish( );

void 	set_reset_state(uint8_t state);
uint8_t get_reset_state();
void 	console_handler(uint8_t* rx_buf, size_t cur_indx);


#endif /* __CONSOLE_H */