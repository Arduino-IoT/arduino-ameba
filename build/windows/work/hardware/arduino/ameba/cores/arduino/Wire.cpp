/*
 * TwoWire.h - TWI/I2C library for Arduino Due
 * Copyright (c) 2011 Cristian Maglie <c.maglie@bug.st>.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "arduino.h"

extern "C" {
#include <string.h>
}


#include "Wire.h"

#if 1 //NeoJou


#else
static inline bool TWI_FailedAcknowledge(Twi *pTwi) {
	return pTwi->TWI_SR & TWI_SR_NACK;
}

static inline bool TWI_WaitTransferComplete(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
			return false;

		if (--_timeout == 0)
			return false;
	}
	return true;
}

static inline bool TWI_WaitByteSent(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_TXRDY) != TWI_SR_TXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
			return false;

		if (--_timeout == 0)
			return false;
	}

	return true;
}

static inline bool TWI_WaitByteReceived(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_RXRDY) != TWI_SR_RXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
			return false;

		if (--_timeout == 0)
			return false;
	}

	return true;
}

static inline bool TWI_STATUS_SVREAD(uint32_t status) {
	return (status & TWI_SR_SVREAD) == TWI_SR_SVREAD;
}

static inline bool TWI_STATUS_SVACC(uint32_t status) {
	return (status & TWI_SR_SVACC) == TWI_SR_SVACC;
}

static inline bool TWI_STATUS_GACC(uint32_t status) {
	return (status & TWI_SR_GACC) == TWI_SR_GACC;
}

static inline bool TWI_STATUS_EOSACC(uint32_t status) {
	return (status & TWI_SR_EOSACC) == TWI_SR_EOSACC;
}

static inline bool TWI_STATUS_NACK(uint32_t status) {
	return (status & TWI_SR_NACK) == TWI_SR_NACK;
}
#endif

//NeoJou

#if 1


extern "C" {
extern
_LONG_CALL_ u32
DiagPrintf(const char *fmt, ...);

extern u32 ConfigDebugErr;

#define 	_DBG_MISC_          	0x40000000

}

TwoWire::TwoWire()
{
	DiagPrintf("TwoWire initializing \r\n");
}

void TwoWire::setpin(PinName SDA_Pin, PinName SCL_Pin )
{
	this->SDA_pin = SDA_Pin;
	this->SCL_pin = SCL_Pin;
}

void TwoWire::setI2CMaster(i2c_t *pObj )
{
	this->pI2Cmaster = pObj;
}

i2c_t * TwoWire::getI2CMaster(void)
{
	return this->pI2Cmaster;
}

void TwoWire::begin(void) {
	this->rxBufferIndex = 0;
	this->rxBufferLength = 0;
	this->txAddress = 0;
	this->txBufferLength = 0;
	this->srvBufferIndex = 0; 
	this->srvBufferLength = 0;
	this->status = UNINITIALIZED;
	this->twiClock = this->TWI_CLOCK;

	
	i2c_init(this->pI2Cmaster, (PinName)this->SDA_pin, (PinName)this->SCL_pin); 	
    i2c_frequency(this->pI2Cmaster,this->twiClock);
	
	status = MASTER_IDLE;
}

void TwoWire::setClock(uint32_t frequency) {
	this->twiClock = frequency;
    i2c_frequency(this->pI2Cmaster,this->twiClock);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
	int readed=0;
	
	if (quantity > BUFFER_LENGTH)
		quantity = BUFFER_LENGTH;

	// perform blocking read into buffer
	readed = i2c_read(this->pI2Cmaster, (int)address, (char*)&this->rxBuffer[0], (int)quantity, (int)sendStop);

	// i2c_read error;
	if ( readed != quantity ) {
		DiagPrintf("requestFrom: readed=%d, quantity=%d : ERROR\n", readed, quantity);
		//i2c_reset(this->pI2Cmaster);
		//this->begin();
		return readed;
	}
	
	// set rx buffer iterator vars
	this->rxBufferIndex = 0;
	this->rxBufferLength = readed;

	return readed;
}

#else //original

TwoWire::TwoWire(Twi *_twi, void(*_beginCb)(void)) :
	twi(_twi), rxBufferIndex(0), rxBufferLength(0), txAddress(0),
			txBufferLength(0), srvBufferIndex(0), srvBufferLength(0), status(
					UNINITIALIZED), onBeginCallback(_beginCb), twiClock(TWI_CLOCK) {
}

void TwoWire::begin(void) {
	if (onBeginCallback)
		onBeginCallback();

	// Disable PDC channel
	twi->TWI_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

	TWI_ConfigureMaster(twi, twiClock, VARIANT_MCK);
	status = MASTER_IDLE;
}

void TwoWire::begin(uint8_t address) {
	if (onBeginCallback)
		onBeginCallback();

	// Disable PDC channel
	twi->TWI_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

	TWI_ConfigureSlave(twi, address);
	status = SLAVE_IDLE;
	TWI_EnableIt(twi, TWI_IER_SVACC);
	//| TWI_IER_RXRDY | TWI_IER_TXRDY	| TWI_IER_TXCOMP);
}

void TwoWire::begin(int address) {
	begin((uint8_t) address);
}

void TwoWire::setClock(uint32_t frequency) {
	twiClock = frequency;
	TWI_SetClock(twi, twiClock, VARIANT_MCK);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
	if (quantity > BUFFER_LENGTH)
		quantity = BUFFER_LENGTH;

	// perform blocking read into buffer
	int readed = 0;
	TWI_StartRead(twi, address, 0, 0);
	do {
		// Stop condition must be set during the reception of last byte
		if (readed + 1 == quantity)
			TWI_SendSTOPCondition( twi);

		TWI_WaitByteReceived(twi, RECV_TIMEOUT);
		rxBuffer[readed++] = TWI_ReadByte(twi);
	} while (readed < quantity);
	TWI_WaitTransferComplete(twi, RECV_TIMEOUT);

	// set rx buffer iterator vars
	rxBufferIndex = 0;
	rxBufferLength = readed;

	return readed;
}
#endif

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
	return requestFrom((uint8_t) address, (uint8_t) quantity, (uint8_t) true);
}

uint8_t TwoWire::requestFrom(int address, int quantity) {
	return requestFrom((uint8_t) address, (uint8_t) quantity, (uint8_t) true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop) {
	return requestFrom((uint8_t) address, (uint8_t) quantity, (uint8_t) sendStop);
}

void TwoWire::beginTransmission(uint8_t address) {
	this->status = MASTER_SEND;

	// save address of target and empty buffer
	this->txAddress = address;
	this->txBufferLength = 0;
}

void TwoWire::beginTransmission(int address) {
	beginTransmission((uint8_t) address);
}

size_t TwoWire::write(uint8_t data) {
	if (this->status == MASTER_SEND) {
		if (this->txBufferLength >= BUFFER_LENGTH)
			return 0;
		this->txBuffer[this->txBufferLength++] = data;
	} else {
		if (this->srvBufferLength >= BUFFER_LENGTH)
			return 0;
		this->srvBuffer[srvBufferLength++] = data;
	}
	return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity) {
	if (this->status == MASTER_SEND) {
		for (size_t i = 0; i < quantity; ++i) {
			if (this->txBufferLength >= BUFFER_LENGTH)
				return i;
			this->txBuffer[this->txBufferLength++] = data[i];
		}
	} else {
		for (size_t i = 0; i < quantity; ++i) {
			if (this->srvBufferLength >= BUFFER_LENGTH)
				return i;
			this->srvBuffer[this->srvBufferLength++] = data[i];
		}
	}
	return quantity;
}

int TwoWire::available(void) {
	return rxBufferLength - rxBufferIndex;
}

int TwoWire::read(void) {
	if (rxBufferIndex < rxBufferLength)
		return rxBuffer[rxBufferIndex++];
	return -1;
}


int TwoWire::peek(void) {
	if (rxBufferIndex < rxBufferLength)
		return rxBuffer[rxBufferIndex];
	return -1;
}

void TwoWire::flush(void) {
	// Do nothing, use endTransmission(..) to force
	// data transfer.
}

//	This provides backwards compatibility with the original
//	definition, and expected behaviour, of endTransmission
//
uint8_t TwoWire::endTransmission(void)
{
	return endTransmission(true);
}


//NeoJou

#if 1

uint8_t TwoWire::endTransmission(uint8_t sendStop) {
	uint8_t error = 0;
	
    error = i2c_write(this->pI2Cmaster, (int)this->txAddress, (const char*)&this->txBuffer[0], (int)this->txBufferLength, (int)sendStop);

	txBufferLength = 0;		// empty buffer
	status = MASTER_IDLE;
	return error;
}



#else //original 


//
//	Originally, 'endTransmission' was an f(void) function.
//	It has been modified to take one parameter indicating
//	whether or not a STOP should be performed on the bus.
//	Calling endTransmission(false) allows a sketch to
//	perform a repeated start.
//
//	WARNING: Nothing in the library keeps track of whether
//	the bus tenure has been properly ended with a STOP. It
//	is very possible to leave the bus in a hung state if
//	no call to endTransmission(true) is made. Some I2C
//	devices will behave oddly if they do not see a STOP.
//
uint8_t TwoWire::endTransmission(uint8_t sendStop) {
	uint8_t error = 0;
	// transmit buffer (blocking)
	TWI_StartWrite(twi, txAddress, 0, 0, txBuffer[0]);
	if (!TWI_WaitByteSent(twi, XMIT_TIMEOUT))
		error = 2;	// error, got NACK on address transmit
	
	if (error == 0) {
		uint16_t sent = 1;
		while (sent < txBufferLength) {
			TWI_WriteByte(twi, txBuffer[sent++]);
			if (!TWI_WaitByteSent(twi, XMIT_TIMEOUT))
				error = 3;	// error, got NACK during data transmmit
		}
	}
	
	if (error == 0) {
		TWI_Stop(twi);
		if (!TWI_WaitTransferComplete(twi, XMIT_TIMEOUT))
			error = 4;	// error, finishing up
	}

	txBufferLength = 0;		// empty buffer
	status = MASTER_IDLE;
	return error;
}



void TwoWire::onReceive(void(*function)(int)) {
	onReceiveCallback = function;
}

void TwoWire::onRequest(void(*function)(void)) {
	onRequestCallback = function;
}

void TwoWire::onService(void) {
	// Retrieve interrupt status
	uint32_t sr = TWI_GetStatus(twi);

	if (status == SLAVE_IDLE && TWI_STATUS_SVACC(sr)) {
		TWI_DisableIt(twi, TWI_IDR_SVACC);
		TWI_EnableIt(twi, TWI_IER_RXRDY | TWI_IER_GACC | TWI_IER_NACK
				| TWI_IER_EOSACC | TWI_IER_SCL_WS | TWI_IER_TXCOMP);

		srvBufferLength = 0;
		srvBufferIndex = 0;

		// Detect if we should go into RECV or SEND status
		// SVREAD==1 means *master* reading -> SLAVE_SEND
		if (!TWI_STATUS_SVREAD(sr)) {
			status = SLAVE_RECV;
		} else {
			status = SLAVE_SEND;

			// Alert calling program to generate a response ASAP
			if (onRequestCallback)
				onRequestCallback();
			else
				// create a default 1-byte response
				write((uint8_t) 0);
		}
	}

	if (status != SLAVE_IDLE) {
		if (TWI_STATUS_TXCOMP(sr) && TWI_STATUS_EOSACC(sr)) {
			if (status == SLAVE_RECV && onReceiveCallback) {
				// Copy data into rxBuffer
				// (allows to receive another packet while the
				// user program reads actual data)
				for (uint8_t i = 0; i < srvBufferLength; ++i)
					rxBuffer[i] = srvBuffer[i];
				rxBufferIndex = 0;
				rxBufferLength = srvBufferLength;

				// Alert calling program
				onReceiveCallback( rxBufferLength);
			}

			// Transfer completed
			TWI_EnableIt(twi, TWI_SR_SVACC);
			TWI_DisableIt(twi, TWI_IDR_RXRDY | TWI_IDR_GACC | TWI_IDR_NACK
					| TWI_IDR_EOSACC | TWI_IDR_SCL_WS | TWI_IER_TXCOMP);
			status = SLAVE_IDLE;
		}
	}

	if (status == SLAVE_RECV) {
		if (TWI_STATUS_RXRDY(sr)) {
			if (srvBufferLength < BUFFER_LENGTH)
				srvBuffer[srvBufferLength++] = TWI_ReadByte(twi);
		}
	}

	if (status == SLAVE_SEND) {
		if (TWI_STATUS_TXRDY(sr) && !TWI_STATUS_NACK(sr)) {
			uint8_t c = 'x';
			if (srvBufferIndex < srvBufferLength)
				c = srvBuffer[srvBufferIndex++];
			TWI_WriteByte(twi, c);
		}
	}
}

#if WIRE_INTERFACES_COUNT > 0
static void Wire_Init(void) {
	pmc_enable_periph_clk(WIRE_INTERFACE_ID);
	PIO_Configure(
			g_APinDescription[PIN_WIRE_SDA].pPort,
			g_APinDescription[PIN_WIRE_SDA].ulPinType,
			g_APinDescription[PIN_WIRE_SDA].ulPin,
			g_APinDescription[PIN_WIRE_SDA].ulPinConfiguration);
	PIO_Configure(
			g_APinDescription[PIN_WIRE_SCL].pPort,
			g_APinDescription[PIN_WIRE_SCL].ulPinType,
			g_APinDescription[PIN_WIRE_SCL].ulPin,
			g_APinDescription[PIN_WIRE_SCL].ulPinConfiguration);

	NVIC_DisableIRQ(WIRE_ISR_ID);
	NVIC_ClearPendingIRQ(WIRE_ISR_ID);
	NVIC_SetPriority(WIRE_ISR_ID, 0);
	NVIC_EnableIRQ(WIRE_ISR_ID);
}

TwoWire Wire = TwoWire(WIRE_INTERFACE, Wire_Init);

void WIRE_ISR_HANDLER(void) {
	Wire.onService();
}
#endif

#if WIRE_INTERFACES_COUNT > 1
static void Wire1_Init(void) {
	pmc_enable_periph_clk(WIRE1_INTERFACE_ID);
	PIO_Configure(
			g_APinDescription[PIN_WIRE1_SDA].pPort,
			g_APinDescription[PIN_WIRE1_SDA].ulPinType,
			g_APinDescription[PIN_WIRE1_SDA].ulPin,
			g_APinDescription[PIN_WIRE1_SDA].ulPinConfiguration);
	PIO_Configure(
			g_APinDescription[PIN_WIRE1_SCL].pPort,
			g_APinDescription[PIN_WIRE1_SCL].ulPinType,
			g_APinDescription[PIN_WIRE1_SCL].ulPin,
			g_APinDescription[PIN_WIRE1_SCL].ulPinConfiguration);

	NVIC_DisableIRQ(WIRE1_ISR_ID);
	NVIC_ClearPendingIRQ(WIRE1_ISR_ID);
	NVIC_SetPriority(WIRE1_ISR_ID, 0);
	NVIC_EnableIRQ(WIRE1_ISR_ID);
}

TwoWire Wire1 = TwoWire(WIRE1_INTERFACE, Wire1_Init);

void WIRE1_ISR_HANDLER(void) {
	Wire1.onService();
}

#endif //NeoJou


#endif
