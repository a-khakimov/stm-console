
#include "fifo.h"


#define BUF_SIZE 1024

struct FIFO {
	uint8_t buf[ BUF_SIZE ];
	uint32_t tail;
	uint32_t head;
} fifo ;

uint32_t FIFO_Size() {
	return ( sizeof( fifo.buf ) / sizeof( fifo.buf[0] ) );
}

void FIFO_Flush() {
	fifo.tail = 0;
	fifo.head = 0;
	memset( &fifo.buf, 0, FIFO_Size() );
}

int FIFO_DataSize() {
	return ( fifo.head - fifo.tail );
}

void FIFO_Push( uint8_t value ) {
	fifo.buf[ fifo.head ] = value;
	fifo.head++;
	if( fifo.head >= BUF_SIZE )
		fifo.head = 0;
}

uint32_t FIFO_Front( ) {
	if( FIFO_DataSize() )
		return fifo.buf[ fifo.tail ];
}

void FIFO_Pop() {
	fifo.tail++;
	if( fifo.tail >= BUF_SIZE )
		fifo.tail = 0;
}

uint8_t FIFO_FrontPop( ) {
	uint32_t temp = 0;
	if( FIFO_DataSize() ) {
		temp = fifo.buf[ fifo.tail ];
		FIFO_Pop();
		return temp;
	}
	else
		return 0;		
}

