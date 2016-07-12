#include <SPI.h>
#include "nRF24L01.h"
#include "CX10.h"

#include "pitches.h"

#define SPEAKER_PIN 3

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3,NOTE_G3, NOTE_A3, NOTE_G3,0, NOTE_B3, NOTE_C4};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4,4,4,4,4 };

void playMelody()
{
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(SPEAKER_PIN, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(SPEAKER_PIN);
  }
}



#define CE_PIN  9
#define CSN_PIN 10

uint8_t txid[4] = { 0x0b, 0x06, 0x34, 0x12 };
// uint8_t txid[4] = { 0xB7, 0x58, 0x66, 0x33 }; // my own


nRF24 radio(CE_PIN, CSN_PIN);
CX10_TX tx(radio);

uint16_t throttle;
uint8_t flags;

int raw_throttle;
int a0, a1, a2, a3;
int a0min, a0max;
int a1min, a1max;
int a2min, a2max;
int a3min, a3max;

void calibrate()
{
  a0min = 900; a0max=100;
  a1min = 900; a1max=100;
  a2min = 900; a2max=100;
  a3min = 900; a3max=100;
}

void initInput()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  calibrate();
}

#define STICK_RANGE 1000
#define STICK_BASE  1000
bool readInput()
{
  bool changed = false;
  long a;
  a = analogRead(A0);
  raw_throttle = a;
  if (a < a0min) a0min = a;
  if (a > a0max) a0max = a;
  a = (a-a0min)*STICK_RANGE/(a0max-a0min)+STICK_BASE;
  if (a != a0) { changed = true; a0 = a; }

  a = analogRead(A1);
  if (a < a1min) a1min = a;
  if (a > a1max) a1max = a;
  a = (a-a1min)*STICK_RANGE/(a1max-a1min)+STICK_BASE;
  if (a != a1) { changed = true; a1 = a; }

  a = analogRead(A2);
  if (a < a2min) a2min = a;
  if (a > a2max) a2max = a;
  a = (a-a2min)*STICK_RANGE/(a2max-a2min)+STICK_BASE;
  if (a != a2) { changed = true; a2 = a; }

  a = analogRead(A3);
  if (a < a3min) a3min = a;
  if (a > a3max) a3max = a;
  a = (a-a3min)*STICK_RANGE/(a3max-a3min)+STICK_BASE;
  if (a != a3) { changed = true; a3 = a; }
  return changed;
}

int last_report;
void setup() 
{
//  playMelody();
  initInput();
  readInput();
  Serial.begin(115200);
//  while (!Serial) ;
  tx.setTXId(txid);  
  tx.begin();
  throttle = 1000;

  Serial.write("Reading status\n");
  uint8_t res = radio.read_register(STATUS);
  Serial.write("Result: ");
  Serial.print(res);
  Serial.write("\n");
  last_report = micros();
} 

int counter = 0;
int direction = 1;
bool bind = true;
bool calibrated = false;
void loop() 
{
  bool changed = readInput();
  if (false) {
//  if (changed) {
    if (micros() > last_report + 1000) {
      Serial.write("sticks: ");
      Serial.print(a0); Serial.write(" ");
      Serial.print(a1); Serial.write(" ");
      Serial.print(a2); Serial.write(" ");
      Serial.print(a3); Serial.write("\n");
      last_report = micros();
    }
  }
  if (bind) {
    throttle = a0;
    flags = 1;
    /* Auto bind in 2.5 sec after turning on
    counter += direction;
    if (direction > 0) {
      if (counter > 256) direction = -1;
    } else {
      if (counter < 0) {
        direction = 1;
        counter = 0;
        bind = false;
        flags = 0;
        Serial.write("Bound\n");
      }
    } */
    if (direction > 0) {
      if (a0max-a0min > STICK_RANGE/2 && raw_throttle < a0max-20) direction = -1;
    } else {
      if (raw_throttle <= a0min+5) {
        playMelody();
        direction = 1;
        counter = 0;
        bind = false;
        flags = 2 << 8; // high sticks mode
        Serial.write("Bound\n");
        Serial.write("a0min "); Serial.print(a0min);
        Serial.write(" a0max "); Serial.print(a0max);
        Serial.write("\na1min "); Serial.print(a1min);
        Serial.write(" a1max "); Serial.print(a1max);
        Serial.write("\na2min "); Serial.print(a2min);
        Serial.write(" a2max "); Serial.print(a2max);
        Serial.write("\na3min "); Serial.print(a3min);
        Serial.write(" a3max "); Serial.print(a3max);
      }
    }
  }
  int t = micros();
//  int future_us = t + tx.command(1000, 1500, 1500, 1500, 1);
  int future_us = t + tx.command(a0, a1, a2, a3, flags);
  while (micros() < future_us) {}
}

