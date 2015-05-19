/*
 * Copyright (c) 2012 by Cristian Maglie <c.maglie@bug.st>
 * DAC library for Arduino Due.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef DAC1_INCLUDED
#define DAC1_INCLUDED



#ifdef __cplusplus // only for C++

#include "Arduino.h"


extern "C" {
#include "hal_dac.h"

typedef struct DACC_struct {
    PSAL_DAC_HND 	pDACVeriHnd;
	PSAL_DAC_HND_PRIV   pSalDACHNDPriv;
	PSAL_DAC_MNGT_ADPT  pSalDACMngtAdpt;
	PHAL_GDMA_ADAPTER   pHALDACGdmaAdpt;
	PHAL_GDMA_OP		  pHALDACGdmaOp;
	PIRQ_HANDLE		  pIrqHandleDACGdma;
} DACC;

}

const int DAC_pin0 = 0;

#define MAX_BUFFER_SIZE (1024)

//typedef void (*OnTransmitEnd_CB)(void *data);

class DACClass1
{
public:
	
	DACClass1(){};
    void begin(uint32_t period);
	void send8(uint8_t* buffer, unsigned int len, unsigned int repeat=1);
	void send16(uint16_t* buffer, unsigned int len, unsigned int repeat=1);
	void send(uint32_t* buffer, unsigned int len, unsigned int repeat=1);

	void send8_from_11K_to_250K(uint8_t* buffer, unsigned int len);
//	void send16_from_8K_to_250K(uint16_t* buffer, unsigned int len);
	void send16_freq(uint16_t* buffer, unsigned int len, int freq);
	uint16_t transform_dac_val(uint16_t val);
	uint16_t transform_dac_val_to_bit12(uint16_t val8, uint bits);
	
//	DACClass(Dacc *_dac, uint32_t _dacId, IRQn_Type _isrId) :
//		dac(_dac), dacId(_dacId), isrId(_isrId), cb(NULL) { };
//	void begin(uint32_t period);
//	void end();
//	bool canQueue();
//	size_t queueBuffer(const uint32_t *buffer, size_t size);
//	uint32_t *getCurrentQueuePointer();
//	void setOnTransmitEnd_CB(OnTransmitEnd_CB _cb, void *data);
//	void onService();

//	void enableInterrupts()  { NVIC_EnableIRQ(isrId); };
//	void disableInterrupts() { NVIC_DisableIRQ(isrId); };

protected:
    void dacc_init(void);
	uint16_t MAXDACVAL = 0x7E0;
	uint16_t MINDACVAL = 0x820;

    uint16_t dac_buffer[2][MAX_BUFFER_SIZE];
    unsigned int dac_buffer_size = 0;
    unsigned int dac_buffer_page=0;

	void transform_buffer8(uint16_t* pBuffer, uint8_t* in_Buffer8, unsigned int len);
	//uint16_t wav_transform_to_bit12(uint16_t val8, uint bits);
    unsigned int transform_buffer8_from_11K_to_250K(uint16_t* pBuffer, uint8_t* in_Buffer8, unsigned int len);
	unsigned int transform_buffer16_to_250K(uint16_t* pBuffer, uint16_t* in_Buffer, unsigned int len, unsigned int from_bps);

 
	
private:
    DACC dac;
	SAL_DAC_TRANSFER_BUF	DACTxBuf;

//	Dacc *dac;
//	uint32_t dacId;
//	IRQn_Type isrId;
//	OnTransmitEnd_CB cb;
//	void *cbData;
};

extern DACClass1 DAC0;

#endif // only for C++

#endif
