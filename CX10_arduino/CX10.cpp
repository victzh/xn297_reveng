#include "CX10.h"
#include "nRF24L01.h"

uint8_t freq_hopping[4] = { 0x16, 0x33, 0x40, 0x0e };
const uint8_t rx_tx_addr[5] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };

void CX10_TX::setTXId(uint8_t txid_[4])
{
  txid[0] = txid_[0];
  txid[1] = txid_[1];
  txid[2] = txid_[2];
  txid[3] = txid_[3];
  
  for (int i = 0; i < 4; ++i) {
    rf_channels[i] = freq_hopping[i];
  }
}

void CX10_TX::init_xn297()
{
  // Check for XN297
  if (!is_xn297) {
    Serial.write("Checking for XN297\n");
    uint8_t buf[5];
    radio.read_register(0x19, buf, 5);
    for (int i = 0; i < 5; ++i) {
      if (buf[i] != 0) {
        is_xn297 = true;
        break;
      }
    }
  }
  if (is_xn297) {
    Serial.write("XN297!\n");
//#define USE_XN297_DATASHEET
#ifdef USE_XN297_DATASHEET
    // DEMOD_CAL
    radio.write_register(0x19, (const uint8_t *) "\x0B\xDF\x00\xA7\x03", 5);
    // RF_CAL
    radio.write_register(0x1e, (const uint8_t *) "\xCA\x9A\xB0\x61\x83\x2B\x95", 7);
    // BB_CAL
    radio.write_register(0x1f, (const uint8_t *) "\x7F\x84\x67\x9C\x20", 5);
#else
    radio.write_register(0x19, (const uint8_t *) "\x0b\xdf\xc4\xa7\x03", 5); 
    radio.write_register(0x1e, (const uint8_t *) "\xc9\x9a\xb0\x61\xbb\xab\x9c", 7); 
    radio.write_register(0x1f, (const uint8_t *) "\x4c\x84\x67\x9c\x20", 5); 
#endif
  }
}

void CX10_TX::init_beken()
{
  if (is_xn297) return;
  // Check for Beken BK2421/BK2423 chip
  // It is done by using Beken specific activate code, 0x53
  // and checking that status register changed appropriately
  // There is no harm to run it on nRF24L01 because following
  // closing activate command changes state back even if it
  // does something on nRF24L01
  radio.activate(0x53); // magic for BK2421 bank switch
  uint8_t r_status = radio.read_register(STATUS);
  bool is_beken = r_status & 0x80;
  Serial.write("Try to switch banks "); Serial.print(r_status); Serial.write("\n");
  if (is_beken) {
    Serial.write("BK2421!\n");
    radio.write_register(0x00, (const uint8_t *) "\x40\x4B\x01\xE2", 4);
    radio.write_register(0x01, (const uint8_t *) "\xC0\x4B\x00\x00", 4);
    radio.write_register(0x02, (const uint8_t *) "\xD0\xFC\x8C\x02", 4);
    radio.write_register(0x03, (const uint8_t *) "\xF9\x00\x39\x21", 4);
    radio.write_register(0x04, (const uint8_t *) "\xC1\x96\x9A\x1B", 4);
    radio.write_register(0x05, (const uint8_t *) "\x24\x06\x7F\xA6", 4);
    radio.write_register(0x0C, (const uint8_t *) "\x00\x12\x73\x00", 4);
    radio.write_register(0x0D, (const uint8_t *) "\x46\xB4\x80\x00", 4);
    radio.write_register(0x0E, (const uint8_t *) "\x41\x10\x04\x82\x20\x08\x08\xF2\x7D\xEF\xFF", 11);
    radio.write_register(0x04, (const uint8_t *) "\xC7\x96\x9A\x1B", 4);
    radio.write_register(0x04, (const uint8_t *) "\xC1\x96\x9A\x1B", 4);
  }
  radio.activate(0x53); // switch bank back
}

// XN297 emulation
const uint8_t scramble[] = {
  0xe3, 0xb1, 0x4b, 0xea, 0x85, 0xbc, 0xe5, 0x66,
  0x0d, 0xae, 0x8c, 0x88, 0x12, 0x69, 0xee, 0x1f,
  0xc7, 0x62, 0x97, 0xd5, 0x0b, 0x79, 0xca, 0xcc,
  0x1b, 0x5d, 0x19, 0x10, 0x24, 0xd3, 0xdc, 0x3f,
  0x8e, 0xc5, 0x2f};

void CX10_TX::setTXAddr(const uint8_t* addr, int len)
{
  if (len > 5) len = 5;
  if (len < 3) len = 3;
  if (is_xn297) {
    uint8_t buf[] = { 0, 0, 0, 0, 0 };
    memcpy(buf, addr, len);
    radio.write_register(SETUP_AW, len-2);
    radio.write_register(TX_ADDR, addr, 5);
    radio.write_register(RX_ADDR_P0, addr, 5);
  } else {
    uint8_t buf[] = { 0x55, 0x0F, 0x71, 0x0C, 0x00 }; // bytes for XN297 preamble 0xC710F55 (28 bit)
    radio.write_register(SETUP_AW, 2);
    radio.write_register(TX_ADDR, buf, 5);
    radio.write_register(RX_ADDR_P0, buf, 5);
    tx_addr_len = len;
    memcpy(tx_addr, addr, len);
  }
}

static uint8_t bit_reverse(uint8_t b_in)
{
  uint8_t b_out = 0;
  for (int i = 0; i < 8; ++i) {
    b_out = (b_out << 1) | (b_in & 1);
    b_in >>= 1;
  }
  return b_out;
}

const static uint16_t polynomial = 0x1021;
const static uint16_t initial    = 0xb5d2;
const static uint16_t xorout     = 0x9ba7;
static uint16_t crc16_update(uint16_t crc, unsigned char a)
{
  crc ^= a << 8;
  for (int i = 0; i < 8; ++i) {
    if (crc & 0x8000) {
      crc = (crc << 1) ^ polynomial;
    } else {
      crc = crc << 1;
    }
  }
  return crc;
}

void CX10_TX::txSend(const uint8_t* msg, int len)
{
  uint8_t packet[32];
  if (is_xn297) {
    radio.write_payload(msg, len);
  } else {
    for (int i = 0; i < tx_addr_len; ++i) {
      packet[i] = tx_addr[tx_addr_len-i-1] ^ scramble[i];
    }

    for (int i = 0; i < len; ++i) {
      // bit-reverse bytes in packet
      uint8_t b_out = bit_reverse(msg[i]);
      packet[tx_addr_len+i] = b_out ^ scramble[tx_addr_len+i];
    }
    int crc_ind = tx_addr_len+len;
    uint16_t crc = initial;
    for (int i = 0; i < crc_ind; ++i) {
      crc = crc16_update(crc, packet[i]);
    }
    crc ^= xorout;
    packet[crc_ind++] = crc >> 8;
    packet[crc_ind++] = crc & 0xff;
    radio.write_payload(packet, crc_ind);
  }
}

void CX10_TX::begin()
{
  delay(100);
  radio.begin();
  
  init_xn297();
  

  radio.flush_tx();
  radio.flush_rx();

  radio.write_register(FIFO_STATUS, 0x00);
  radio.write_register(EN_AA, 0x00);
  radio.write_register(EN_RXADDR, 0x01);

//  radio.write_register(SETUP_AW, 0x03); // 5-byte address
  setTXAddr(rx_tx_addr, 5);

  radio.write_register(RF_CH, 0x02);
  radio.write_register(SETUP_RETR, 0x00);
  radio.write_register(RX_PW_P0, 0x0F); // accept 15-byte packets at pipe 0
  radio.write_register(RF_SETUP, 0x07); // 0x07 - 1Mbps, 0dBm power, LNA high gaim
  radio.activate(0x73);
  radio.write_register(DYNPD, 0);
  radio.write_register(FEATURE, 0);

  radio.write_register(STATUS, 0x70);
  
  delay(100);

  init_beken();

  if (is_xn297) {
    radio.write_register(CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP));
  } else {
    radio.write_register(CONFIG, _BV(PWR_UP));
  }
//  radio.flush_tx();
//  radio.flush_rx();
  delay(100);
//  delayMicroseconds(150);
//  packet_sent = true;
  rf_ch_num = 0;
  radio.flush_tx();
//  radio.ce(HIGH);
  delayMicroseconds(30);
  // It saves power to turn off radio after the transmission,
  // so as long as we have pins to do so, it is wise to turn
  // it back.
//  radio.ce(LOW);
}

//static uint8_t debug_counter = 0;
int CX10_TX::command(uint16_t throttle, uint16_t rudder, uint16_t elevator, uint16_t aileron, uint8_t flags)
{
  int packet_len = 15;
  uint8_t packet[15];
  if (flags & 0xff) {
    // binding
    packet[0] = 0xAA;
  } else {
    // regular packet
    packet[0] = 0x55;
  }
  // TX id
  packet[1] = txid[0];
  packet[2] = txid[1];
  packet[3] = txid[2];
  packet[4] = txid[3];

  packet[5] = aileron & 0xff;
  packet[6] = (aileron >> 8) & 0xff;
  packet[7] = elevator & 0xff;
  packet[8] = (elevator >> 8) & 0xff;
  packet[9] = throttle & 0xff;
  packet[10] = (throttle >> 8) & 0xff;
  packet[11] = rudder & 0xff;
  packet[12] = (rudder >> 8) & 0xff;
  packet[13] = (flags >> 8) & 0xff;
  packet[14] = 0;//flags & 0xff;
  
  if (packet_sent && !is_xn297) {
    bool report_done = false;
    if  (!(radio.read_register(STATUS) & _BV(TX_DS))) { Serial.write("Waiting for radio\n"); report_done = true; }
    while (!(radio.read_register(STATUS) & _BV(TX_DS))) ;
    radio.write_register(STATUS, _BV(TX_DS));
    if (report_done) Serial.write("Done\n");
  }
  packet_sent = true;

#if 0 // DEBUG code for figuring out XN297 encoding
  uint8_t addr[5];
  for (int i = 0; i < 5; ++i) addr[i] = 0;
  setTXAddr(addr, 3);
  for (int i = 0; i < 32; ++i) packet[i] = 0;
  packet_len = 2;
#endif

// Differential attack on the CRC described in
// http://www.cosc.canterbury.ac.nz/greg.ewing/essays/CRC-Reverse-Engineering.html
#if 0
  // generate packets with address length 3 and 2 bytes message,
  // then 4 and 1 byte
  int addr_len = 3 + (debug_counter & 1);     // 3, 4, 3, 4
  int shift = (debug_counter >> 1) & 7;       // 0, 0, 1, 1 .. 7, 7, 0, 0
  int pattern = (debug_counter >> 4)*2 + 1;   // 1, 1, 1, 1 .. 1, 1, 3, 3 .. 1, 1
  uint8_t test_byte = (pattern << shift);
  uint8_t addr[5] = { 0, 0, 0, 0, 0};
  // Check that the check sum is invariant to the composition of radio packet
  // test byte is either the last byte (first in interface) of the address (if addr_len == 4)
  // or first byte of data packet
  addr[0] = scramble[addr_len-1] ^ test_byte;
  int test_byte_in_addr = addr_len - 3;
  for (int i = test_byte_in_addr; i < addr_len; ++i)
    addr[i] = scramble[addr_len-i-1];
  setTXAddr(addr, addr_len);
  packet_len = 5 - addr_len;
  packet[0] = bit_reverse(scramble[addr_len] ^ test_byte);
  for (int i = (1 - test_byte_in_addr); i < packet_len; ++i)
    packet[i] = bit_reverse(scramble[addr_len+i]);
  debug_counter = ++debug_counter & 0x1f;
#endif

  uint8_t rf_ch;
  if (flags) {
    rf_ch = 0x02;
  } else {
    rf_ch = rf_channels[rf_ch_num];
    rf_ch_num++; if (rf_ch_num >= 4) rf_ch_num = 0;
  }
//  Serial.print(rf_ch); Serial.write("\n");
  radio.write_register(RF_CH, rf_ch);
  radio.write_register(STATUS, 0x70);
  radio.flush_tx();
//  radio.write_payload(packet, packet_len);
  txSend(packet, packet_len);
  radio.ce(HIGH);
  delayMicroseconds(30);
  radio.ce(LOW);
  if (flags & 0xff)
    return 1500; // 1.5ms between packets
  else
    return 10000; // 10ms
//  byte_to_send += 1;
//  return byte_to_send == 0 ? 6000 : 1500; // debug delay
}

