#include <SPI.h>
#include "nRF24L01.h"

/****************************************************************************/

void nRF24::csn(int mode) {
  digitalWrite(csn_pin, mode);
}

/****************************************************************************/

void nRF24::ce(int level)
{
  digitalWrite(ce_pin, level);
}

/****************************************************************************/

void nRF24::begin() {
  // Initialize SPI bus
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.begin();

  // Initialize pins
  pinMode(ce_pin,OUTPUT);
  pinMode(csn_pin,OUTPUT);

  digitalWrite(ce_pin, LOW);
  digitalWrite(csn_pin, HIGH);

  delay(5);

  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

uint8_t nRF24::read_register(uint8_t reg, uint8_t* buf, uint8_t len)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    *buf++ = SPI.transfer(0xff);

  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t nRF24::read_register(uint8_t reg)
{
  csn(LOW);
  SPI.transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
  uint8_t result = SPI.transfer(0xff);

  csn(HIGH);
  return result;
}

/****************************************************************************/

uint8_t nRF24::write_register(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    SPI.transfer(*buf++);

  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t nRF24::write_register(uint8_t reg, uint8_t value)
{
  uint8_t status;

//  IF_SERIAL_DEBUG(printf_P(PSTR("write_register(%02x,%02x)\r\n"),reg,value));

  csn(LOW);
  status = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  SPI.transfer(value);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t nRF24::write_payload(const void* buf, uint8_t len)
{
  uint8_t status;

  const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

//  uint8_t data_len = min(len,payload_size);
//  uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;
  
  //printf("[Writing %u bytes %u blanks]",data_len,blank_len);
  
  csn(LOW);
  status = SPI.transfer( W_TX_PAYLOAD );
  while ( len-- )
    SPI.transfer(*current++);
//  while ( blank_len-- )
//    SPI.transfer(0);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t nRF24::read_payload(void* buf, uint8_t len)
{
  uint8_t status;
  uint8_t* current = reinterpret_cast<uint8_t*>(buf);

  uint8_t data_len = min(len,payload_size);
  uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;
  
  //printf("[Reading %u bytes %u blanks]",data_len,blank_len);
  
  csn(LOW);
  status = SPI.transfer( R_RX_PAYLOAD );
  while ( data_len-- )
    *current++ = SPI.transfer(0xff);
  while ( blank_len-- )
    SPI.transfer(0xff);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t nRF24::flush_rx(void)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( FLUSH_RX );
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t nRF24::flush_tx(void)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( FLUSH_TX );
  csn(HIGH);

  return status;
}

/****************************************************************************/

void nRF24::activate(uint8_t code)
{
  csn(LOW);
  SPI.transfer(ACTIVATE);
  SPI.transfer(code);
  csn(HIGH);
}

