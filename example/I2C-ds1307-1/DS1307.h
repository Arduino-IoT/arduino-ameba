#ifndef _DS1307_H_
#define _DS1307_H_

#define DS1307_SEC 0
#define DS1307_MIN 1
#define DS1307_HR 2
#define DS1307_DOW 3
#define DS1307_DATE 4
#define DS1307_MTH 5
#define DS1307_YR 6

#define DS1307_BASE_YR 2000

#define DS1307_SQW1HZ  7
#define DS1307_SQW4KHZ 8
#define DS1307_SQW8KHZ 9
#define DS1307_SQW32KHZ 10

//#define DS1307_CTRL_ID 0x68 // B1101000  //DS1307

 // Define register bit masks
#define DS1307_CLOCKHALT 0x80 //B10000000

#define DS1307_LO_BCD  0x0F //B00001111
#define DS1307_HI_BCD  0xF0 //B11110000

#define DS1307_HI_SEC  0x70 // B01110000
#define DS1307_HI_MIN  0x70 //B01110000
#define DS1307_HI_HR   0x30 //B00110000
#define DS1307_LO_DOW  0x07 //B00000111
#define DS1307_HI_DATE 0x30 //B00110000
#define DS1307_HI_MTH  0x30 //B00110000
#define DS1307_HI_YR   0xF0 //B11110000


// Prescaler
#define DS1307_LOW_BIT		0x0 //B00000000
#define DS1307_HIGH_BIT		0x80 //B10000000
#define DS1307_SQW1HZ_BIT	0x10 //B00010000
#define DS1307_SQW4KHZ_BIT	0x11 //B00010001
#define DS1307_SQW8KHZ_BIT	0x12 //B00010010
#define DS1307_SQW32KHZ_BIT	0x13 //B00010011


extern void DS1307_init();
extern void DS1307_get(int *rtc, boolean refresh);
extern void DS1307_stop(void);
extern void DS1307_save(void);
extern void DS1307_set_data(int c, int v);
extern void DS1307_start(void);
extern void DS1307_SetOutput(int c);

#endif
