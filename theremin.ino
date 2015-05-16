/*****************************************************************************
 Title:             Theremin.ino
 Author:            Anna Cristina Karingal
 Created on:        April 8, 2015
 Description:       An Arduino theremin using the Capsense Library to sense input
                     Changes instrument depending on buttons pressed

 Last Modified:     May 15, 2015

 *****************************************************************************/

// LCD Libraries
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// Serial Library for Midi
#include <SoftwareSerial.h>

// CapSense Library
#include <CapacitiveSensor.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
SoftwareSerial VS1053_MIDI(0, 2);

// Backlight colors
#define ON 0x1
#define OFF 0x0
#define WHITE 0x7

// MIDI Pins
#define VS1054_RX 2 // Connects to the RX pin on VS1053
#define VS1053_RESET 9 // Connects to the RESET pin on VS1053

// MIDI Actions
#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0

// MIDI Sounds
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_MELODY 0x79

// Set up instrments
struct instrument {
  int midiRef;
  String instrName;

  instrument (int mr, String n): midiRef(mr), instrName(n) { };
};

instrument OCARINA(80, "Ocarina");
instrument GLOCK(10, "Glockenspiel");
instrument CELLO(43, "Cello");
instrument ALL_INSTRUMENTS[] = {OCARINA,  GLOCK, CELLO};
int NUM_OF_INSTRUMENTS = 3;
int i = 0; // To iterate through sounds
long prev = 0;

// Set up sensors: Pins 4 & 13 are receive sensor
CapacitiveSensor volSensor = CapacitiveSensor(9,10);
CapacitiveSensor pitchSensor = CapacitiveSensor(7, 8);

void setup() {

  pitchSensor.set_CS_AutocaL_Millis(0xFFFFFFFF);

  // Display welcome screen
  lcd.begin(16, 2); // Sets num of cols & rows
  lcd.print("Arduino Theremin");
  lcd.setBacklight(WHITE);

  // Set up MIDI
  VS1053_MIDI.begin(31250);

  // Set Reset Pin (D9) to output
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  digitalWrite(VS1053_RESET, HIGH);

  // Initialize midi sound
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, OCARINA.midiRef);
  midiSetChannelVolume(0, 127);

  delay(2000);

}

void loop() {

  lcd.setCursor(0, 1);

  // Get sensor pitch input and output as midi sound
  long pitch = pitchSensor.capacitiveSensor(10);
  midiNoteOn(0, pitch, 127);

  // Get volume sensor input and output as midi volume
  long vol = volSensor.capacitiveSensor(10);
  long diff = abs(vol - prev);

  // So that volume doesn't jump all over the place
  // Only change volume if difference < 5
  if (diff < 10) {
    midiSetChannelVolume(0, vol);
  }
  prev = vol;

  // Play note for 15ms before turning off
  delay(15);
  midiNoteOff(0, pitch, 127);

  // Scan for button input.
  // Change instrument if up/down button pressed and display on LCD
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    lcd.clear();
    lcd.setCursor(0, 0);
    if (buttons & BUTTON_UP) {
      if (i < NUM_OF_INSTRUMENTS - 1) {
        i++;
      }
      else {
        i = 0;
      }
    }
    if (buttons & BUTTON_DOWN) {
      if (i > 0) {
        i--;
      }
      else {
        i = NUM_OF_INSTRUMENTS - 1;
      }
    }

    lcd.print(ALL_INSTRUMENTS[i].instrName);
    midiSetInstrument(0, ALL_INSTRUMENTS[i].midiRef);
    delay(100);
  }

}


/**
 *  Functions to write to midi channel
 */
void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --;
  if (inst > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);
  VS1053_MIDI.write(inst);
}

void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}


