#ifndef FIFO__H
#define FIFO__H

#include <inttypes.h>

uint32_t 	FIFO_Size();
void 		FIFO_Flush();
int 		FIFO_DataSize();
void 		FIFO_Push( uint8_t value );
uint32_t 	FIFO_Front();
void 		FIFO_Pop();
uint8_t 	FIFO_FrontPop();



#endif //FIFO__H
