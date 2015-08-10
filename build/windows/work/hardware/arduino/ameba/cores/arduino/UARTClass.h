/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _UART_CLASS_
#define _UART_CLASS_

#include "HardwareSerial.h"
#include "RingBuffer.h"
#include "WString.h"

#include "hal_irqn.h"

// NeoJou
//#include "serial_api.h"


class UARTClass: public Stream//public HardwareSerial
{
//  public: 
//	serial_t	  sobj;

  protected:
    RingBuffer *_rx_buffer ;

    //Uart* _pUart ;
    //void* _pUart;
    IRQn_Type _dwIrq ;
    //uint32_t _dwId ;

  public:
    //UARTClass( Uart* pUart, IRQn_Type dwIrq, uint32_t dwId, RingBuffer* pRx_buffer ) ;
    //UARTClass( void* pUart, IRQn_Type dwIrq, uint32_t dwId, RingBuffer* pRx_buffer ) ;
    UARTClass(IRQn_Type dwIrq, RingBuffer* pRx_buffer ) ;

    void begin( const uint32_t dwBaudRate ) ;
    void end( void ) ;
    int available( void ) ;
    int peek( void ) ;
    int read( void ) ;
    void flush( void ) ;
    size_t write( const uint8_t c ) ;

	//serial_t* getSerialObject( void);

    void IrqHandler( void ) ;

    using Print::write ; // pull in write(str) and write(buf, size) from Print

    operator bool() { return true; }; // UART always active

  	
#if 0
// print
  private:
	  size_t printNumber(unsigned long, uint8_t);
	

 public:
	size_t write(const uint8_t *buffer, size_t size);

	size_t write(const char *buffer, size_t size) {
	  return write((const uint8_t *)buffer, size);
	}
	
    size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }

	size_t print(const char *str);

	size_t print(char c);

	size_t print(unsigned int n, int base);

	size_t println(void);

	size_t println(const char *c);

	size_t println(char c);

	size_t println(unsigned int num, int base);
#endif
};

#endif // _UART_CLASS_
