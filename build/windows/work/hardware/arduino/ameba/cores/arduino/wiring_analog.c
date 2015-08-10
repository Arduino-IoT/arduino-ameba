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

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// NeoJou
//
#include "basic_types.h"

#include "Pinmap.h"
#include "hal_pinmux.h"

static const PinMap PinMap_PWM[] = {
    {PB_4,  RTL_PIN_PERI(PWM0, 0, S0), RTL_PIN_FUNC(PWM0, S0)},
    {PB_5,  RTL_PIN_PERI(PWM1, 1, S0), RTL_PIN_FUNC(PWM1, S0)},
    {PB_6,  RTL_PIN_PERI(PWM2, 2, S0), RTL_PIN_FUNC(PWM2, S0)},
    {PB_7,  RTL_PIN_PERI(PWM3, 3, S0), RTL_PIN_FUNC(PWM3, S0)},

    {PC_0,  RTL_PIN_PERI(PWM0, 0, S1), RTL_PIN_FUNC(PWM0, S1)},
    {PC_1,  RTL_PIN_PERI(PWM1, 1, S1), RTL_PIN_FUNC(PWM1, S1)},
    {PC_2,  RTL_PIN_PERI(PWM2, 2, S1), RTL_PIN_FUNC(PWM2, S1)},
    {PC_3,  RTL_PIN_PERI(PWM3, 3, S1), RTL_PIN_FUNC(PWM3, S1)},

    {PD_3,  RTL_PIN_PERI(PWM0, 0, S2), RTL_PIN_FUNC(PWM0, S2)},
    {PD_4,  RTL_PIN_PERI(PWM1, 1, S2), RTL_PIN_FUNC(PWM1, S2)},
    {PD_5,  RTL_PIN_PERI(PWM2, 2, S2), RTL_PIN_FUNC(PWM2, S2)},
    {PD_6,  RTL_PIN_PERI(PWM3, 3, S2), RTL_PIN_FUNC(PWM3, S2)},

    {PE_0,  RTL_PIN_PERI(PWM0, 0, S3), RTL_PIN_FUNC(PWM0, S3)},
    {PE_1,  RTL_PIN_PERI(PWM1, 1, S3), RTL_PIN_FUNC(PWM1, S3)},
    {PE_2,  RTL_PIN_PERI(PWM2, 2, S3), RTL_PIN_FUNC(PWM2, S3)},
    {PE_3,  RTL_PIN_PERI(PWM3, 3, S3), RTL_PIN_FUNC(PWM3, S3)},

    {NC,    NC,     0}
};



typedef struct pwmout_s pwmout_t;

static pwmout_t pwm_pins[14];

static void pwmout_init         (pwmout_t* obj, PinName pin);



void pwmout_init(pwmout_t* obj, PinName pin) 
{
    uint32_t peripheral;
    u32 pwm_idx;
    u32 pin_sel;

    
    // Get the peripheral name from the pin and assign it to the object
    peripheral = pinmap_peripheral(pin, PinMap_PWM);

    if ( peripheral == NC ) {
        return;
    }

    pwm_idx = RTL_GET_PERI_IDX(peripheral);
    pin_sel = RTL_GET_PERI_SEL(peripheral);

    obj->pwm_idx = pwm_idx;
    obj->pin_sel = pin_sel;
    obj->period = 0;
    obj->pulse = 0;
    HAL_Pwm_Init(pwm_idx, pin_sel);
	obj->period = 20000;
    HAL_Pwm_SetDuty(obj->pwm_idx, obj->period, obj->pulse);
    HAL_Pwm_Enable(pwm_idx);
}




//
// Arduino
//

static int _readResolution = 10;
static int _writeResolution = 8;

void analogReadResolution(int res) {
	_readResolution = res;
}

void analogWriteResolution(int res) {
	_writeResolution = res;
}

static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to) {
	if (from == to)
		return value;
	if (from > to)
		return value >> (from-to);
	else
		return value << (to-from);
}

eAnalogReference analog_reference = AR_DEFAULT;

void analogReference(eAnalogReference ulMode)
{
	analog_reference = ulMode;
}

static const float ADC_slope1 = (3.12)/(3410.0-674.0);
static const float ADC_slope2 = (3.3-3.12)/(3454.0-3410.0);
uint32_t analogRead_for_random()
{
    return analogin_read_for_random(&adc1);
}

uint32_t analogRead(uint32_t ulPin)
{
  uint32_t ulValue = 0;
  uint32_t ulChannel;
  uint16_t ret = 0;
  float	   voltage;
  float	   adc_value;

#if 1 //NeoJou
  switch ( ulPin ) {
  	case A0: // A1 == A0
		ret = analogin_read_u16(&adc2);
		break;
	case A2:
		ret = analogin_read_u16(&adc3);
		break;
	default:
		DiagPrintf("ulPin is not for adc \n");
		return 0;
  }

  ret >>= 4;
  if (ret < 674) {
  	 voltage = 0;
  } else if ( ret > 3410){
     voltage = (float)(ret - 3410)*ADC_slope2 + 3.12;
  } else { 
	 voltage = (float)(ret-674)*ADC_slope1;
  }

  // Arduino analogRead()
  // input : 0~5V
  // 10 bit : 0 ~1023
  ret = round(1023.0*voltage/5.0);
  if ( ret > 1023 ) ret = 1023;
  return ret;
  //return analogin_read_u16(&adc2);
#else
  if (ulPin < A0)
    ulPin += A0;

  ulChannel = g_APinDescription[ulPin].ulADCChannelNumber ;

#if defined __SAM3U4E__
	switch ( g_APinDescription[ulPin].ulAnalogChannel )
	{
		// Handling ADC 10 bits channels
		case ADC0 :
		case ADC1 :
		case ADC2 :
		case ADC3 :
		case ADC4 :
		case ADC5 :
		case ADC6 :
		case ADC7 :
			// Enable the corresponding channel
			adc_enable_channel( ADC, ulChannel );

			// Start the ADC
			adc_start( ADC );

			// Wait for end of conversion
			while ((adc_get_status(ADC) & ADC_SR_DRDY) != ADC_SR_DRDY)
				;

			// Read the value
			ulValue = adc_get_latest_value(ADC);
			ulValue = mapResolution(ulValue, 10, _readResolution);

			// Disable the corresponding channel
			adc_disable_channel( ADC, ulChannel );

			// Stop the ADC
			//      adc_stop( ADC ) ; // never do adc_stop() else we have to reconfigure the ADC each time
			break;

		// Handling ADC 12 bits channels
		case ADC8 :
		case ADC9 :
		case ADC10 :
		case ADC11 :
		case ADC12 :
		case ADC13 :
		case ADC14 :
		case ADC15 :
			// Enable the corresponding channel
			adc12b_enable_channel( ADC12B, ulChannel );

			// Start the ADC12B
			adc12b_start( ADC12B );

			// Wait for end of conversion
			while ((adc12b_get_status(ADC12B) & ADC12B_SR_DRDY) != ADC12B_SR_DRDY)
				;

			// Read the value
			ulValue = adc12b_get_latest_value(ADC12B) >> 2;
			ulValue = mapResolution(ulValue, 12, _readResolution);

			// Stop the ADC12B
			//      adc12_stop( ADC12B ) ; // never do adc12_stop() else we have to reconfigure the ADC12B each time

			// Disable the corresponding channel
			adc12b_disable_channel( ADC12B, ulChannel );
			break;

		// Compiler could yell because we don't handle DAC pins
		default :
			ulValue=0;
			break;
	}
#endif

#if defined __SAM3X8E__ || defined __SAM3X8H__
	static uint32_t latestSelectedChannel = -1;
	switch ( g_APinDescription[ulPin].ulAnalogChannel )
	{
		// Handling ADC 12 bits channels
		case ADC0 :
		case ADC1 :
		case ADC2 :
		case ADC3 :
		case ADC4 :
		case ADC5 :
		case ADC6 :
		case ADC7 :
		case ADC8 :
		case ADC9 :
		case ADC10 :
		case ADC11 :

			// Enable the corresponding channel
			if (ulChannel != latestSelectedChannel) {
				adc_enable_channel( ADC, ulChannel );
				if ( latestSelectedChannel != (uint32_t)-1 )
					adc_disable_channel( ADC, latestSelectedChannel );
				latestSelectedChannel = ulChannel;
			}

			// Start the ADC
			adc_start( ADC );

			// Wait for end of conversion
			while ((adc_get_status(ADC) & ADC_ISR_DRDY) != ADC_ISR_DRDY)
				;

			// Read the value
			ulValue = adc_get_latest_value(ADC);
			ulValue = mapResolution(ulValue, ADC_RESOLUTION, _readResolution);

			break;

		// Compiler could yell because we don't handle DAC pins
		default :
			ulValue=0;
			break;
	}
#endif

#endif // NeoJou
	return ulValue;
}


static uint8_t PWMEnabled = 0;
//static uint8_t pinEnabled[PINS_COUNT];

void analogOutputInit(void) {
	uint8_t i;
//	for (i=0; i<PINS_COUNT; i++)
//		pinEnabled[i] = 0;
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite(uint32_t ulPin, uint32_t ulValue) {

#if 1 //NeoJou

	pwmout_t *pObj;

	if ( ulPin < 0 || ulPin > 13 ) return;

	/* Handle */
	if ( g_APinDescription[ulPin].ulPinType != PIO_PWM )
	{
		pwmout_init(&pwm_pins[ulPin], g_APinDescription[ulPin].pinname);
		g_APinDescription[ulPin].ulPinType = PIO_PWM;
	}

	pObj = &pwm_pins[ulPin];
    pObj->pulse = pObj->period * ulValue / 256;
    HAL_Pwm_SetDuty(pObj->pwm_idx, pObj->period, pObj->pulse);


#else
	uint32_t attr = g_APinDescription[ulPin].ulPinAttribute;

	if ((attr & PIN_ATTR_ANALOG) == PIN_ATTR_ANALOG) {
		EAnalogChannel channel = g_APinDescription[ulPin].ulADCChannelNumber;
		if (channel == DA0 || channel == DA1) {
			uint32_t chDACC = ((channel == DA0) ? 0 : 1);
			if (dacc_get_channel_status(DACC_INTERFACE) == 0) {
				/* Enable clock for DACC_INTERFACE */
				pmc_enable_periph_clk(DACC_INTERFACE_ID);

				/* Reset DACC registers */
				dacc_reset(DACC_INTERFACE);

				/* Half word transfer mode */
				dacc_set_transfer_mode(DACC_INTERFACE, 0);

				/* Power save:
				 * sleep mode  - 0 (disabled)
				 * fast wakeup - 0 (disabled)
				 */
				dacc_set_power_save(DACC_INTERFACE, 0, 0);
				/* Timing:
				 * refresh        - 0x08 (1024*8 dacc clocks)
				 * max speed mode -    0 (disabled)
				 * startup time   - 0x10 (1024 dacc clocks)
				 */
				dacc_set_timing(DACC_INTERFACE, 0x08, 0, 0x10);

				/* Set up analog current */
				dacc_set_analog_control(DACC_INTERFACE, DACC_ACR_IBCTLCH0(0x02) |
											DACC_ACR_IBCTLCH1(0x02) |
											DACC_ACR_IBCTLDACCORE(0x01));
			}

			/* Disable TAG and select output channel chDACC */
			dacc_set_channel_selection(DACC_INTERFACE, chDACC);

			if ((dacc_get_channel_status(DACC_INTERFACE) & (1 << chDACC)) == 0) {
				dacc_enable_channel(DACC_INTERFACE, chDACC);
			}

			// Write user value
			ulValue = mapResolution(ulValue, _writeResolution, DACC_RESOLUTION);
			dacc_write_conversion_data(DACC_INTERFACE, ulValue);
			while ((dacc_get_interrupt_status(DACC_INTERFACE) & DACC_ISR_EOC) == 0);
			return;
		}
	}

	if ((attr & PIN_ATTR_PWM) == PIN_ATTR_PWM) {
		ulValue = mapResolution(ulValue, _writeResolution, PWM_RESOLUTION);

		if (!PWMEnabled) {
			// PWM Startup code
		    pmc_enable_periph_clk(PWM_INTERFACE_ID);
		    PWMC_ConfigureClocks(PWM_FREQUENCY * PWM_MAX_DUTY_CYCLE, 0, VARIANT_MCK);
			PWMEnabled = 1;
		}

		uint32_t chan = g_APinDescription[ulPin].ulPWMChannel;
		if (!pinEnabled[ulPin]) {
			// Setup PWM for this pin
			PIO_Configure(g_APinDescription[ulPin].pPort,
					g_APinDescription[ulPin].ulPinType,
					g_APinDescription[ulPin].ulPin,
					g_APinDescription[ulPin].ulPinConfiguration);
			PWMC_ConfigureChannel(PWM_INTERFACE, chan, PWM_CMR_CPRE_CLKA, 0, 0);
			PWMC_SetPeriod(PWM_INTERFACE, chan, PWM_MAX_DUTY_CYCLE);
			PWMC_SetDutyCycle(PWM_INTERFACE, chan, ulValue);
			PWMC_EnableChannel(PWM_INTERFACE, chan);
			pinEnabled[ulPin] = 1;
		}

		PWMC_SetDutyCycle(PWM_INTERFACE, chan, ulValue);
		return;
	}

	if ((attr & PIN_ATTR_TIMER) == PIN_ATTR_TIMER) {
		// We use MCLK/2 as clock.
		const uint32_t TC = VARIANT_MCK / 2 / TC_FREQUENCY;

		// Map value to Timer ranges 0..255 => 0..TC
		ulValue = mapResolution(ulValue, _writeResolution, TC_RESOLUTION);
		ulValue = ulValue * TC;
		ulValue = ulValue / TC_MAX_DUTY_CYCLE;

		// Setup Timer for this pin
		ETCChannel channel = g_APinDescription[ulPin].ulTCChannel;
		static const uint32_t channelToChNo[] = { 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2 };
		static const uint32_t channelToAB[]   = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
		static Tc *channelToTC[] = {
			TC0, TC0, TC0, TC0, TC0, TC0,
			TC1, TC1, TC1, TC1, TC1, TC1,
			TC2, TC2, TC2, TC2, TC2, TC2 };
		static const uint32_t channelToId[] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8 };
		uint32_t chNo = channelToChNo[channel];
		uint32_t chA  = channelToAB[channel];
		Tc *chTC = channelToTC[channel];
		uint32_t interfaceID = channelToId[channel];

		if (!TCChanEnabled[interfaceID]) {
			pmc_enable_periph_clk(TC_INTERFACE_ID + interfaceID);
			TC_Configure(chTC, chNo,
				TC_CMR_TCCLKS_TIMER_CLOCK1 |
				TC_CMR_WAVE |         // Waveform mode
				TC_CMR_WAVSEL_UP_RC | // Counter running up and reset when equals to RC
				TC_CMR_EEVT_XC0 |     // Set external events from XC0 (this setup TIOB as output)
				TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
				TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR);
			TC_SetRC(chTC, chNo, TC);
		}
		if (ulValue == 0) {
			if (chA)
				TC_SetCMR_ChannelA(chTC, chNo, TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR);
			else
				TC_SetCMR_ChannelB(chTC, chNo, TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR);
		} else {
			if (chA) {
				TC_SetRA(chTC, chNo, ulValue);
				TC_SetCMR_ChannelA(chTC, chNo, TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);
			} else {
				TC_SetRB(chTC, chNo, ulValue);
				TC_SetCMR_ChannelB(chTC, chNo, TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_SET);
			}
		}
		if (!pinEnabled[ulPin]) {
			PIO_Configure(g_APinDescription[ulPin].pPort,
					g_APinDescription[ulPin].ulPinType,
					g_APinDescription[ulPin].ulPin,
					g_APinDescription[ulPin].ulPinConfiguration);
			pinEnabled[ulPin] = 1;
		}
		if (!TCChanEnabled[interfaceID]) {
			TC_Start(chTC, chNo);
			TCChanEnabled[interfaceID] = 1;
		}
		return;
	}

	// Defaults to digital write
	pinMode(ulPin, OUTPUT);
	ulValue = mapResolution(ulValue, _writeResolution, 8);
	if (ulValue < 128)
		digitalWrite(ulPin, LOW);
	else
		digitalWrite(ulPin, HIGH);
#endif
}

#ifdef __cplusplus
}
#endif
