/*
 * Copyright (c) 2012 by Cristian Maglie <c.maglie@bug.st>
 * DAC library for Arduino Due.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include <DAC1.h>


#if 1


extern "C" {

#include "hal_dac.h"

}


void DACClass1::dacc_init() {
	
    this->dac.pDACVeriHnd = NULL;
    this->dac.pSalDACHNDPriv = NULL;
	this->dac.pSalDACMngtAdpt = NULL;
	this->dac.pHALDACGdmaAdpt = NULL;
	this->dac.pHALDACGdmaOp = NULL;
	this->dac.pIrqHandleDACGdma = NULL;
}

void DACClass1::begin(uint32_t period) 
{
	int pin = DAC_pin0;
	
	// init DAC
	
    PSAL_DAC_HND        pSalDACHND      = NULL;


    dacc_init();

    /* Check the user define setting and the given index */
    if (RtkDACIdxChk(pin)) {
        return;
    }

    /* Invoke RtkI2CGetMngtAdpt to get the I2C SAL management adapter pointer */
    this->dac.pSalDACMngtAdpt = RtkDACGetMngtAdpt(pin);

    /* Assign the private SAL handle to public SAL handle */
    pSalDACHND      = &(this->dac.pSalDACMngtAdpt->pSalHndPriv->SalDACHndPriv);
    /* Assign the internal HAL initial data pointer to the SAL handle */
    pSalDACHND->pInitDat    = this->dac.pSalDACMngtAdpt->pHalInitDat;
    /* Assign the internal user callback pointer to the SAL handle */
    pSalDACHND->pUserCB     = this->dac.pSalDACMngtAdpt->pUserCB;
    
    this->dac.pDACVeriHnd = &(this->dac.pSalDACMngtAdpt->pSalHndPriv->SalDACHndPriv);

	//

	this->dac.pDACVeriHnd->DevNum	  = pin;
	RtkDACLoadDefault(this->dac.pDACVeriHnd);
	this->dac.pDACVeriHnd->pInitDat->DACDataRate	  =   DAC_DATA_RATE_250K;
	//pDACVeriHnd->pInitDat->DACDataRate		  =   DAC_DATA_RATE_10K;
	this->dac.pDACVeriHnd->pInitDat->DACEndian		  =   DAC_DATA_ENDIAN_LITTLE;
	this->dac.pDACVeriHnd->pInitDat->DACBurstSz 	  =   10;

	/* To Get the SAL_I2C_MNGT_ADPT Pointer */
	pSalDACHND = this->dac.pDACVeriHnd;
	this->dac.pSalDACHNDPriv  = CONTAINER_OF(this->dac.pDACVeriHnd, SAL_DAC_HND_PRIV, SalDACHndPriv);
	this->dac.pSalDACMngtAdpt = CONTAINER_OF(this->dac.pSalDACHNDPriv->ppSalDACHnd, SAL_DAC_MNGT_ADPT, pSalHndPriv);

	this->dac.pHALDACGdmaAdpt	  = this->dac.pSalDACMngtAdpt->pHalGdmaAdp;
	this->dac.pHALDACGdmaOp 	  = this->dac.pSalDACMngtAdpt->pHalGdmaOp;  
	this->dac.pIrqHandleDACGdma   = this->dac.pSalDACMngtAdpt->pIrqGdmaHnd;

	RtkDACInit(this->dac.pDACVeriHnd);
	this->dac.pHALDACGdmaAdpt->MuliBlockCunt	  = 2;
	this->dac.pHALDACGdmaAdpt->MaxMuliBlock 	  = 2;//MaxLlp;
}



void DACClass1::transform_buffer8(uint16_t* pBuffer, uint8_t* in_Buffer8, unsigned int len)
{
	for (int i = 0; i < len; i++) {
	   pBuffer[i] = transform_dac_val_to_bit12(in_Buffer8[i], 8);
	   
	}
}


#if 1

#else
unsigned int DACClass1::transform_buffer8_from_11K_to_250K(uint16_t* pBuffer, uint8_t* in_Buffer, unsigned int len)
{
    unsigned int new_len;
	double steps, val1, val2, pre_val;
	unsigned int new_len_max = len * 250 / 11;
	unsigned int pre_pos =0, new_pos=0, in_pos=0; 

    if ( len <= 0 ) return 0;

	pBuffer[0] = wav_transform_to_bit12(in_Buffer[0], 8);
	if ( len < 1 ) {
		return 1;
	}
	
	val1 = (double)(transform_dac_val_to_bit12(in_Buffer[0], 8));
	val2 = (double)(transform_dac_val_to_bit12(in_Buffer[1], 8));
	pre_pos = 0;
	steps = (val2-val1)*11.0/250.0; pre_val = val1;
	for (int i = 1; i < new_len_max-1; i++) {
	   in_pos = i * 11 / 250;
	   if ( in_pos == pre_pos ) {
	   	 pre_val = pre_val+steps;
	   } else {
		   val1 = (double)(transform_dac_val_to_bit12(in_Buffer[pre_pos], 8));
		   val2 = (double)(transform_dac_val_to_bit12(in_Buffer[in_pos], 8));
		   steps = (val2-val1)*11.0/250.0; pre_val = val1;
		   pre_pos = in_pos;
	   }	   
	   pBuffer[i] = (uint16_t)(transform_dac_val((uint16_t)round(pre_val)));
	}
	
	pBuffer[new_len_max-1] = (uint16_t)(transform_dac_val((uint16_t)round(val2)));	
	return new_len_max;
}
#endif

unsigned int DACClass1::transform_buffer16_to_250K(uint16_t* pBuffer, uint16_t* in_Buffer, unsigned int len, unsigned int from_bps) {
	
    unsigned int new_len;
	double steps, val1, val2, pre_val;
	unsigned int new_len_max = len * 250 / from_bps ;
	unsigned int pre_pos =0, new_pos=0, in_pos=0; 

    if ( len <= 0 ) return 0;

	val1 = (double)(in_Buffer[0]);
	if ( len == 1 ) {
		pBuffer[0] = transform_dac_val((uint16_t)round(val1));	
		return 1;
	}
	
	val2 = (double)(in_Buffer[1]);
	pre_pos = 0;
	steps = (val2-val1)*(double)(from_bps)/250.0; 
	pre_val = val1;
	for (int i = 1; i < new_len_max-1; i++) {
	   in_pos = i * from_bps / 250;
	   if ( in_pos == pre_pos ) {
	   	 pre_val = pre_val+steps;
	   } else {
		   val1 = (double)(in_Buffer[pre_pos]);
		   val2 = (double)(in_Buffer[in_pos]);
		   steps = (val2-val1)*(double)(from_bps)/250.0; 
		   pre_val = val1;
		   pre_pos = in_pos;
	   }
	   pBuffer[i] = (uint16_t)(transform_dac_val((uint16_t)round(pre_val)));
	}
	
	pBuffer[new_len_max-1] = (uint16_t)(transform_dac_val((uint16_t)round(val2)));	
	return new_len_max;
}



void DACClass1::send8_from_11K_to_250K(uint8_t* buffer, unsigned int len)
{
	int new_max_len = MAX_BUFFER_SIZE*11/250;
	int new_len;
	
	if ( len <= new_max_len ) {
		// handle buffer 
		uint16_t *pBuffer = (uint16_t*)(&dac_buffer[dac_buffer_page][0]);
		dac_buffer_page = ( dac_buffer_page + 1 ) % 2;
		
		new_len = transform_buffer8_from_11K_to_250K(pBuffer, buffer, len);
		send((uint32_t*)pBuffer, new_len, 1);
	
	} else {
			send8_from_11K_to_250K(buffer, new_max_len);
			send8_from_11K_to_250K((uint8_t*)(&buffer[new_max_len]), len - new_max_len);
	}
}


void DACClass1::send8(uint8_t* buffer, unsigned int len, unsigned int repeat)
{
	if ( len <= MAX_BUFFER_SIZE ) {
		// handle buffer 
		uint16_t *pBuffer = (uint16_t*)(&dac_buffer[dac_buffer_page][0]);
		dac_buffer_page = ( dac_buffer_page + 1 ) % 2;
		
		transform_buffer8(pBuffer, buffer, len);
		send((uint32_t*)pBuffer, len, repeat);
	
	} else {
		for (int i=0; i<repeat; i++) {
			send8(buffer, MAX_BUFFER_SIZE, 1);
			send8((uint8_t*)(&buffer[MAX_BUFFER_SIZE]), len - MAX_BUFFER_SIZE, 1);
		}
	}
}

#if 0
void DACClass1::send16_from_8K_to_250K(uint16_t* buffer, unsigned int len)
{
	int new_max_len = MAX_BUFFER_SIZE*8/250;
	int new_len;
	
	if ( len <= new_max_len ) {
		// handle buffer 
		uint16_t *pBuffer = (uint16_t*)(&dac_buffer[dac_buffer_page][0]);
		dac_buffer_page = ( dac_buffer_page + 1 ) % 2;
		
		new_len = transform_buffer16_to_250K(pBuffer, buffer, len, 8);
		send((uint32_t*)pBuffer, new_len, 1);
	
	} else {
			send16_from_8K_to_250K(buffer, new_max_len);
			send16_from_8K_to_250K((uint16_t*)(&buffer[new_max_len]), len - new_max_len);
	}
}
#endif

void DACClass1::send16_freq(uint16_t* buffer, unsigned int len, int freq)
{
	int new_max_len = MAX_BUFFER_SIZE* freq/250;
	int new_len;
	
	if ( len <= new_max_len ) {
		// handle buffer 
		uint16_t *pBuffer = (uint16_t*)(&dac_buffer[dac_buffer_page][0]);
		dac_buffer_page = ( dac_buffer_page + 1 ) % 2;
		
		new_len = transform_buffer16_to_250K(pBuffer, buffer, len, freq);
		send((uint32_t*)pBuffer, new_len, 1);
	
	} else {
			send16_freq(buffer, new_max_len, freq);
			send16_freq((uint16_t*)(&buffer[new_max_len]), len - new_max_len, freq);
	}
}


void DACClass1::send16(uint16_t* buffer, unsigned int len, unsigned int repeat)
{
		PSAL_DAC_HND	    pSalDACHND;  
		PHAL_GDMA_ADAPTER	pHALDACGdmaAdpt    = NULL;


	if ( len <= MAX_BUFFER_SIZE ) {

		// handle buffer 
		uint16_t *pBuffer = (uint16_t*)(&dac_buffer[dac_buffer_page][0]);
		dac_buffer_page = ( dac_buffer_page + 1 ) % 2;

		for (int i = 0; i < len; i++) {
		   pBuffer[i] = transform_dac_val(buffer[i]);
		}
		
		send((uint32_t*)pBuffer, len, repeat);
	} else {
		for (int i=0; i<repeat; i++) {
			send16(buffer, MAX_BUFFER_SIZE, 1);
			send16((uint16_t*)(((uint16_t*)(buffer))+MAX_BUFFER_SIZE), len - MAX_BUFFER_SIZE, 1);
		}
	}
}


void DACClass1::send(uint32_t* buffer, unsigned int len, unsigned int repeat)
{
		PSAL_DAC_HND	    pSalDACHND;  
		PHAL_GDMA_ADAPTER	pHALDACGdmaAdpt    = NULL;
		int count = 0;


		if ( len <= MAX_BUFFER_SIZE ) {
			pSalDACHND = this->dac.pDACVeriHnd;
			pHALDACGdmaAdpt = this->dac.pHALDACGdmaAdpt;
		
			while ( pHALDACGdmaAdpt->MuliBlockCunt		< pHALDACGdmaAdpt->MaxMuliBlock ) { // not finish send yet
			  HalDelayUs(1);
			}
		//	  RtkDACWait();
		
			
			pSalDACHND->pTXBuf 			= &this->DACTxBuf;
			pSalDACHND->pTXBuf->DataLen	= len/2;
			pSalDACHND->pTXBuf->pDataBuf	= buffer;
		
			pHALDACGdmaAdpt->MuliBlockCunt		= 1;
			pHALDACGdmaAdpt->MaxMuliBlock		= repeat+1;//MaxLlp;
		
			RtkDACSend(pSalDACHND);
		} else {
			for (int i=0; i<repeat; i++) {
				send(buffer, MAX_BUFFER_SIZE, 1);
				send((uint32_t*)(((uint16_t*)(buffer))+MAX_BUFFER_SIZE), len - MAX_BUFFER_SIZE, 1);
			}
		}

}

uint16_t DACClass1::transform_dac_val(uint16_t val) 
{
  
    if ( val >= 0x800 ) {
      	  val -= 0x800;
    	  if ( val > this->MAXDACVAL ) val = this->MAXDACVAL;
    } else {
    	  val += 0x800;
    	  if ( val < this->MINDACVAL ) val = this->MINDACVAL;
    }
   
   return val;   
}

#if 0
uint16_t DACClass1::wav_transform_to_bit12(uint16_t val8, uint bits) 
{
    uint16_t val;

	if ( bits < 8 ) {
		return 0;
	} else if ( bits <= 12) {
		val = (val8 << (12-bits));
	} else if ( bits <= 16 ) {
		val = (val8 >> (bits-12));
	} else {
		return 0;
	}

	if ( val >= 0x800 && val < this->MINDACVAL ) val = this->MINDACVAL;
	if ( val < 0x800 && val > this->MAXDACVAL) val = this->MAXDACVAL;

	return val;       
}
#endif

uint16_t DACClass1::transform_dac_val_to_bit12(uint16_t val8, uint bits) 
{
    uint16_t val;

	if ( bits < 8 ) {
		return 0;
	} else if ( bits <= 12) {
		val = (val8 << (12-bits));
	} else if ( bits <= 16 ) {
		val = (val8 >> (bits-12));
	} else {
		return 0;
	}

	return transform_dac_val(val);       
}


#else
void DACClass1::begin(uint32_t period) {
	// Enable clock for DAC
	pmc_enable_periph_clk(dacId);

	dacc_reset(dac);

	// Set transfer mode to double word
	dacc_set_transfer_mode(dac, 1);

	// Power save:
	// sleep mode  - 0 (disabled)
	// fast wakeup - 0 (disabled)
	dacc_set_power_save(dac, 0, 0);

	// DAC refresh/startup timings:
	// refresh        - 0x08 (1024*8 dacc clocks)
	// max speed mode -    0 (disabled)
	// startup time   - 0x10 (1024 dacc clocks)
	dacc_set_timing(dac, 0x08, 0, DACC_MR_STARTUP_1024);

	// Flexible channel selection with tags
	dacc_enable_flexible_selection(dac);

	// Set up analog current
	dacc_set_analog_control(dac,
			DACC_ACR_IBCTLCH0(0x02) |
			DACC_ACR_IBCTLCH1(0x02) |
			DACC_ACR_IBCTLDACCORE(0x01));

	// Enable output channels
	dacc_enable_channel(dac, 0);
	dacc_enable_channel(dac, 1);

	// Configure Timer Counter to trigger DAC
	// --------------------------------------
	pmc_enable_periph_clk(ID_TC1);
	TC_Configure(TC0, 1,
		TC_CMR_TCCLKS_TIMER_CLOCK2 |  // Clock at MCR/8
		TC_CMR_WAVE |                 // Waveform mode
		TC_CMR_WAVSEL_UP_RC |         // Counter running up and reset when equals to RC
		TC_CMR_ACPA_SET | TC_CMR_ACPC_CLEAR);
	const uint32_t TC = period / 8;
	TC_SetRA(TC0, 1, TC / 2);
	TC_SetRC(TC0, 1, TC);
	TC_Start(TC0, 1);

	// Configure clock source for DAC (2 = TC0 Output Chan. 1)
	dacc_set_trigger(dac, 2);

	// Configure pins
	PIO_Configure(g_APinDescription[DAC0].pPort,
			g_APinDescription[DAC0].ulPinType,
			g_APinDescription[DAC0].ulPin,
			g_APinDescription[DAC0].ulPinConfiguration);
	PIO_Configure(g_APinDescription[DAC1].pPort,
			g_APinDescription[DAC1].ulPinType,
			g_APinDescription[DAC1].ulPin,
			g_APinDescription[DAC1].ulPinConfiguration);

	// Enable interrupt controller for DAC
	dacc_disable_interrupt(dac, 0xFFFFFFFF);
	NVIC_DisableIRQ(isrId);
	NVIC_ClearPendingIRQ(isrId);
	NVIC_SetPriority(isrId, 0);
	NVIC_EnableIRQ(isrId);
}

void DACClass::end() {
	TC_Stop(TC0, 1);
	NVIC_DisableIRQ(isrId);
	dacc_disable_channel(dac, 0);
	dacc_disable_channel(dac, 1);
}

bool DACClass::canQueue() {
	return (dac->DACC_TNCR == 0);
}

size_t DACClass::queueBuffer(const uint32_t *buffer, size_t size) {
	// Try the first PDC buffer
	if ((dac->DACC_TCR == 0) && (dac->DACC_TNCR == 0)) {
		dac->DACC_TPR = (uint32_t) buffer;
		dac->DACC_TCR = size;
		dac->DACC_PTCR = DACC_PTCR_TXTEN;
		if (cb)
			dacc_enable_interrupt(dac, DACC_IER_ENDTX);
		return size;
	}

	// Try the second PDC buffer
	if (dac->DACC_TNCR == 0) {
		dac->DACC_TNPR = (uint32_t) buffer;
		dac->DACC_TNCR = size;
		dac->DACC_PTCR = DACC_PTCR_TXTEN;
		if (cb)
			dacc_enable_interrupt(dac, DACC_IER_ENDTX);
		return size;
	}

	// PDC buffers full, try again later...
	return 0;
}

void DACClass::setOnTransmitEnd_CB(OnTransmitEnd_CB _cb, void *_data) {
	cb = _cb;
	cbData = _data;
	if (!cb)
		dacc_disable_interrupt(dac, DACC_IDR_ENDTX);
}

void DACClass::onService() {
	uint32_t sr = dac->DACC_ISR;
	if (sr & DACC_ISR_ENDTX) {
		// There is a free slot, enqueue data
		dacc_disable_interrupt(dac, DACC_IDR_ENDTX);
		if (cb)
			cb(cbData);
	}
}

DACClass DAC(DACC_INTERFACE, DACC_INTERFACE_ID, DACC_ISR_ID);

void DACC_ISR_HANDLER(void) {
	DAC.onService();
}
#endif
