#ifndef _SHTC1_H_
#define _SHTC1_H_

typedef unsigned short          uint16_t;

#define NO_ERROR        0x00
#define ACK_ERROR       0x01
#define CHECKSUM_ERROR  0x02
#define NULL_ERROR      0x03

extern int SHTC1_Init(uint16_t *pID);
extern int SHTC1_GetTempAndHumi(float *temp, float *humi);

#endif
