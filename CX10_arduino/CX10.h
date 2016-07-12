#include "Arduino.h"

class nRF24;
class CX10_TX {
  nRF24& radio;
  uint8_t txid[4];
  uint8_t rf_channels[4];
  bool packet_sent;
  bool is_xn297;
  int8_t tx_addr_len;
  uint8_t tx_addr[5];
  uint8_t rf_ch_num;
  void init_xn297();
  void init_beken();
public:
  CX10_TX(nRF24& radio_) :
    radio(radio_)
  {}
  void setTXId(uint8_t txid_[4]);
  void begin();
  void setTXAddr(const uint8_t* addr, int len);
  void txSend(const uint8_t* msg, int len);
  int command(uint16_t throttle, uint16_t rudder, uint16_t elevator, uint16_t aileron, uint8_t flags);
};

