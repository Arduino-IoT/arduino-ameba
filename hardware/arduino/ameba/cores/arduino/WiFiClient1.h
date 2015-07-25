#ifndef wificlient1_h
#define wificlient1_h

#include "Arduino.h"	
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"
#include "TCPSocketConnection.h"

class WiFiClient1 : public Client {

public:
	
  WiFiClient1();

  uint8_t status();
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  //friend class WiFiServer;

  using Print::write;

private:
	TCPSocketConnection _tcpSocket;
	char _readchar;
	bool _readchar_set;
	uint8_t _sock;	 

	uint8_t getFirstSocket();

};

#endif

