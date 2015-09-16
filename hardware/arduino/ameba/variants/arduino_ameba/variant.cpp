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
#include "variant.h"
#include <stdarg.h>

extern "C" {

#include "rtl8195a.h"
#include "hal_irqn.h"
#include "hal_diag.h"
#include "analogin_api.h"
#include "us_ticker_api.h"
#include "rt_os_service.h"

analogin_t   adc1;
analogin_t   adc2;
analogin_t   adc3;

} // extern C

#include "wire.h"
#include "WiFi1.h"


extern "C" {

#include "section_config.h"

void __libc_init_array(void);

/*
 * Pins descriptions
 */
IMAGE2_DATA_SECTION 
PinDescription g_APinDescription[]=
{
  
  {PA_6, NOT_INITIAL, NOT_INITIAL}, //D0
  {PA_7, NOT_INITIAL, NOT_INITIAL}, //D1
  {PA_5, NOT_INITIAL, NOT_INITIAL}, //D2
  {PD_4, NOT_INITIAL, NOT_INITIAL}, //D3 : PWM1
  {PD_5, NOT_INITIAL, NOT_INITIAL}, //D4 : PWM2
  {PA_4, NOT_INITIAL, NOT_INITIAL}, //D5
  {PA_3, NOT_INITIAL, NOT_INITIAL}, //D6
  {PA_2, NOT_INITIAL, NOT_INITIAL}, //D7
  {PB_4, NOT_INITIAL, NOT_INITIAL}, //D8 : PWM0
  {PB_5, NOT_INITIAL, NOT_INITIAL}, //D9 : PWM1
  {PC_0, NOT_INITIAL, NOT_INITIAL}, //D10 : PWM0
  {PC_2, NOT_INITIAL, NOT_INITIAL}, //D11 : PWM2
  {PC_3, NOT_INITIAL, NOT_INITIAL}, //D12 : PWM3
  {PC_1, NOT_INITIAL, NOT_INITIAL}  //D13 : PWM1

};

// uart
void UART_Handler(void);


//i2c
i2c_t i2cmaster_wire1;


} // extern C

// LogUart
IMAGE2_DATA_SECTION 
static RingBuffer rx_buffer0;

UARTClass Serial(UART_LOG_IRQ, &rx_buffer0);

void serialEvent() __attribute__((weak));
void serialEvent() { }

// IT handlers
IMAGE2_TEXT_SECTION 
void UART_Handler(void)
{
  Serial.IrqHandler();
}


static inline void ReRegisterSerial(void)
{
    IRQ_HANDLE          UartIrqHandle;
    
    //4 Register Log Uart Callback function
    UartIrqHandle.Data = NULL;//(u32)&UartAdapter;
    UartIrqHandle.IrqNum = UART_LOG_IRQ; //UART_LOG_IRQ;
    UartIrqHandle.IrqFun = (IRQ_FUN)(UART_Handler);
    UartIrqHandle.Priority = 0;

    
    //4 Register Isr handle
    InterruptUnRegister(&UartIrqHandle); 
    InterruptRegister(&UartIrqHandle); 
}

IMAGE2_TEXT_SECTION 
void init_hal_uart(void)
{
	ReRegisterSerial();
}

//I2C
IMAGE2_DATA_SECTION 
TwoWire Wire1;

IMAGE2_TEXT_SECTION 
void init_hal_i2c(void)
{
	Wire1.setpin(PD_7, PD_6);
	Wire1.setI2CMaster(&i2cmaster_wire1);
}

// adc
IMAGE2_TEXT_SECTION 
void init_hal_adc(void)
{
    analogin_init(&adc1, AD_1);
    analogin_init(&adc2, AD_2);
    analogin_init(&adc3, AD_3);
		
}


// DAC
IMAGE2_DATA_SECTION
DACClass1 DAC0;


// WiFi
IMAGE2_TEXT_SECTION
void init_hal_wifi(void)
{
	WiFi1.init();
}
// ----------------------------------------------------------------------------

IMAGE2_TEXT_SECTION
void serialEventRun(void)
{

  if (Serial.available()) serialEvent();
//  if (Serial1.available()) serialEvent1();
//  if (Serial2.available()) serialEvent2();
//  if (Serial3.available()) serialEvent3();

}


IMAGE2_TEXT_SECTION
void init( void )
{

	// Initialize C library
	__libc_init_array();
	
	init_hal_uart();
	init_hal_i2c();
	init_hal_adc();
	init_hal_wifi();
	us_ticker_init();


	ConfigDebugInfo&= (~(_DBG_GPIO_ | _DBG_UART_));
	ConfigDebugErr&= (~(_DBG_MISC_));

	
}





