
#ifndef __DHT1_h
#define __DHT1_h

#include <Arduino.h>

#define DHT_LIB_VERSION "0.1.13"

#define DHTLIB_OK                0
#define DHTLIB_ERROR_CHECKSUM   -1
#define DHTLIB_ERROR_TIMEOUT    -2
#define DHTLIB_INVALID_VALUE    -999

#define DHTLIB_DHT11_WAKEUP     18
#define DHTLIB_DHT_WAKEUP       1



class DHT1
{
public:
    // return values:
    // DHTLIB_OK
    // DHTLIB_ERROR_CHECKSUM
    // DHTLIB_ERROR_TIMEOUT
    int read(uint8_t pin);

    inline int read22(uint8_t pin) { return read(pin); };

    double humidity;
    double temperature;

private:
    uint8_t bits[5];  // buffer to receive data
    int _readSensor(uint8_t pin, uint8_t wakeupDelay);
};

#endif
//
// END OF FILE
//

