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

#include "arduino.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "UARTClass.h"


extern "C"{

#include "hal_irqn.h"
#include "hal_diag.h"


// Constructors ////////////////////////////////////////////////////////////////
// TODO: NeoJou : need to implement UART class for 
//  when pmc / nvic is ready 

#if 0
#define UART1_TX    PD_3    // QC board J27[8]/EV board J4[19]: UART1, S0
#define UART1_RX    PD_0    // QC board J27[2]/EV board J4[22]: UART1, S0

#define UART0_TX    PC_3    // EV board J6[9]: UART0, S0
#define UART0_RX    PC_0    // EV board J6[122]: UART0, S0
#endif



} // extern C


//UARTClass::UARTClass( Uart* pUart, IRQn_Type dwIrq, uint32_t dwId, RingBuffer* pRx_buffer )
//UARTClass::UARTClass( void* pUart, IRQn_Type dwIrq, uint32_t dwId, RingBuffer* pRx_buffer )
UARTClass::UARTClass(IRQn_Type dwIrq, RingBuffer* pRx_buffer )
{


	_rx_buffer = pRx_buffer ;

//	_pUart=pUart ;
	_dwIrq=dwIrq ;
//	_dwId=dwId ;


}


// Protected Methods //////////////////////////////////////////////////////////////




// Public Methods //////////////////////////////////////////////////////////////


void UARTClass::IrqHandler( void )
{

    uint8_t     data = 0;
    BOOL    	PullMode = _FALSE;
    uint32_t 	IrqEn = DiagGetIsrEnReg();

    DiagSetIsrEnReg(0);

    data = DiagGetChar(PullMode);
	if ( data > 0 ) 
		_rx_buffer->store_char(data);

    DiagSetIsrEnReg(IrqEn);


/*
  uint32_t status = _pUart->UART_SR;

  // Did we receive data ?
  if ((status & UART_SR_RXRDY) == UART_SR_RXRDY)
    _rx_buffer->store_char(_pUart->UART_RHR);

  // Acknowledge errors
  if ((status & UART_SR_OVRE) == UART_SR_OVRE ||
		  (status & UART_SR_FRAME) == UART_SR_FRAME)
  {
	// TODO: error reporting outside ISR
    _pUart->UART_CR |= UART_CR_RSTSTA;
  }
*/
}


void UARTClass::begin( const uint32_t dwBaudRate )
{
//NeoJou
#if 0
	serial_init(&(this->sobj),UART1_TX,UART1_RX);

	serial_baud(&(this->sobj),dwBaudRate);
	serial_format(&(this->sobj), 8, ParityNone, 1);
#endif
/*
  // Configure PMC
  pmc_enable_periph_clk( _dwId ) ;

  // Disable PDC channel
  _pUart->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS ;

  // Reset and disable receiver and transmitter
  _pUart->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS ;

  // Configure mode
  _pUart->UART_MR = UART_MR_PAR_NO | UART_MR_CHMODE_NORMAL ;

  // Configure baudrate (asynchronous, no oversampling)
  _pUart->UART_BRGR = (SystemCoreClock / dwBaudRate) >> 4 ;

  // Configure interrupts
  _pUart->UART_IDR = 0xFFFFFFFF;
  _pUart->UART_IER = UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME;

  // Enable UART interrupt in NVIC
  NVIC_EnableIRQ(_dwIrq);

  // Enable receiver and transmitter
  _pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN ;
*/
}

void UARTClass::end( void )
{

  // clear any received data
  _rx_buffer->_iHead = _rx_buffer->_iTail ;

/*
  // Disable UART interrupt in NVIC
  NVIC_DisableIRQ( _dwIrq ) ;
*/
  // Wait for any outstanding data to be sent
  //flush();

/*
  pmc_disable_periph_clk( _dwId ) ;

 */
}

int UARTClass::available( void )
{
//NeoJou
#if 0
	return serial_readable(&(this->sobj));
#endif
  return (uint32_t)(SERIAL_BUFFER_SIZE + _rx_buffer->_iHead - _rx_buffer->_iTail) % SERIAL_BUFFER_SIZE ;
}

int UARTClass::peek( void )
{

  if ( _rx_buffer->_iHead == _rx_buffer->_iTail )
    return -1 ;

  return _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;

}

int UARTClass::read( void )
{

//  return serial_getc(&(this->sobj));


  // if the head isn't ahead of the tail, we don't have any characters
  if ( _rx_buffer->_iHead == _rx_buffer->_iTail )
    return -1 ;

  uint8_t uc = _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;
  _rx_buffer->_iTail = (unsigned int)(_rx_buffer->_iTail + 1) % SERIAL_BUFFER_SIZE ;
  return uc ;

}

void UARTClass::flush( void )
{

// while ( serial_writable(&(this->sobj)) != 1 );
/*
  // Wait for transmission to complete
  while ((_pUart->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY)
    ;
*/
}

size_t UARTClass::write( const uint8_t uc_data )
{

	HalSerialPutcRtl8195a(uc_data);

//	serial_putc(&(this->sobj), uc_data);
	
/*
  // Check if the transmitter is ready
  while ((_pUart->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY)
    ;

  // Send character
  _pUart->UART_THR = uc_data;
*/
  	return 1;

}


//NeoJou
#if 0
serial_t* UARTClass::getSerialObject( void)
{
	return &(this->sobj);
}


//
// Print
//

size_t UARTClass::write(const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  while (size--) {
    n += write(*buffer++);
  }
  return n;
}

size_t UARTClass::print(const char *str)
{
  return write(str);
}


size_t UARTClass::print(char c)
{
  return write((uint8_t)c);
}

size_t UARTClass::print(unsigned int n, int base)
{
  if (base == 0) return write(n);
  else return printNumber(n, base);
}


size_t UARTClass::println(void)
{
  size_t n = print('\r');
  n += print('\n');
  return n;
}


size_t UARTClass::println(const char *str)
{
  size_t n = print(str);
  n += println();
  return n;
}

size_t UARTClass::println(char c)
{
  size_t n = print(c);
  n += println();
  return n;
}

size_t UARTClass::println(unsigned int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

// Private Methods /////////////////////////////////////////////////////////////

size_t UARTClass::printNumber(unsigned long n, uint8_t base) {
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  // prevent crash if called with base == 1
  if (base < 2) base = 10;

  do {
    unsigned long m = n;
    n /= base;
    char c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);

  return write(str);
}

#endif

