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

struct instrument {
  int midiRef;
  String instrName; 
    
  instrument (int mr, String n): midiRef(mr), instrName(n) { };
};
  
instrument OCARINA(80, "ocarina");
instrument FLUTE(74, "flute"); 
instrument SAX(65, "saxophone");
instrument ALL_INSTRUMENTS[] = {OCARINA, FLUTE, SAX}; 
int NUM_OF_INSTRUMENTS = 3;
int i=0; // To iterate through sounds

// Set up sensors
CapacitiveSensor pitchSensor = CapacitiveSensor(12,13);    

void setup() {
  Serial.begin(9600);
  
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
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);
  
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, OCARINA.midiRef);
  midiSetChannelVolume(0, 127);
  
}

void loop() {
  
  lcd.setCursor(0, 1);
  
 
    long total1 =  pitchSensor.capacitiveSensor(20);

    Serial.print(total1);
    Serial.print('\n');    // print sensor output 1
  
  for (uint8_t n=60; n<69; n++) {
    midiNoteOn(0, n, 127);
    delay(100);
    midiNoteOff(0, n, 127);
  }
  
  // Scan for button input.
  // Change instrument if up/down button pressed and display on LCD
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if (buttons & BUTTON_UP) {
      if (i < NUM_OF_INSTRUMENTS-1){ i++; }
      else { i = 0; } 
    }
    if (buttons & BUTTON_DOWN) {
      if (i>0) { i--; }
      else { i = NUM_OF_INSTRUMENTS-1; }
   }
    
   lcd.print(ALL_INSTRUMENTS[i].instrName);
   midiSetInstrument(0, ALL_INSTRUMENTS[i].midiRef);
  }
  
  delay( 20 );
}


void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
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


