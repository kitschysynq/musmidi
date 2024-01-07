/*
  MIDI note player

  This sketch shows how to use the serial transmit pin (pin 1) to send MIDI note data.
  If this circuit is connected to a MIDI synth, it will play the notes
  F#-0 (0x1E) to F#-5 (0x5A) in sequence.

  The circuit:
  - digital in 1 connected to MIDI jack pin 5
  - MIDI jack pin 2 connected to ground
  - MIDI jack pin 4 connected to +5V through 220 ohm resistor
  - Attach a MIDI cable to the jack, then to a MIDI synth, and play music.

  created 13 Jun 2006
  modified 13 Aug 2012
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Midi
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "footswitch_ssid";
const char *password = "icanhazpassword";

ESP8266WebServer server(80);

// disable midi and send debugging info to serial monitor 115200
// #define SERIAL_DEBUGGING;

const int buttonPin0 = 13;
const int buttonPin1 = 5;
const int buttonPin2 = 4;
const int buttonPin3 = 0;
const int ledPin = BUILTIN_LED;

int buttonState0 = HIGH;
int buttonState1 = HIGH;
int buttonState2 = HIGH;
int buttonState3 = HIGH;
int remoteButtonState0 = HIGH;
int remoteButtonState1 = HIGH;
int prevButtonState0 = HIGH;
int prevButtonState1 = HIGH;
int prevButtonState2 = HIGH;
int prevButtonState3 = HIGH;
int prevRemoteButtonState0 = HIGH;
int prevRemoteButtonState1 = HIGH;
unsigned long debounce1 = 0;
unsigned long debounce2 = 0;
unsigned long debounce3 = 0;
unsigned long debounce4 = 0;
unsigned long remoteDebounce0 = 0;
unsigned long remoteDebounce1 = 0;
const int DEBOUNCE_INTERVAL = 10;

// MIDI Message Types
const uint8_t CC = 0xB0;

// MIDI velocities
const uint8_t velocity_off = 0;
const uint8_t velocity_full = 127;

// MIDI control
const uint8_t control_01 = 1;
const uint8_t control_02 = 2;
const uint8_t control_03 = 3;
const uint8_t control_04 = 4;

// MIDI channel
const uint8_t channel = 1;

void handleSentVar() {
  if (server.hasArg("buttonState0") && server.hasArg("buttonState1")) {  // this is the variable sent from the client
    remoteButtonState0 = server.arg("buttonState0").toInt();
    remoteButtonState1 = server.arg("buttonState1").toInt();
    char buffer[40];
    sprintf(buffer, "remoteButtonState0: %d / remoteButtonState1: %d", remoteButtonState0, remoteButtonState1);
#ifdef SERIAL_DEBUGGING
    // Serial.println(buffer);
#endif
    server.send(200, "text/html", "Data received");
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin0, INPUT_PULLUP);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  server.on("/data/", HTTP_GET, handleSentVar);  // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.begin();

#ifdef SERIAL_DEBUGGING
  // Set up a new SoftwareSerial object
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("HTTP server started");
#else
  // Set MIDI baud rate:
  Serial.begin(31250);
#endif
}

void sendMidi(uint8_t messageType, uint8_t channel, uint8_t dataByte1, uint8_t dataByte2) {
  // Adjust zero-based MIDI channel
  channel--;

  // Create MIDI status byte
  uint8_t statusByte = 0b10000000 | messageType | channel;

  // Send MIDI status and data
#ifdef SERIAL_DEBUGGING
  char buffer[40];
  sprintf(buffer, "status: %d / control: %d / velocity: %d", statusByte, dataByte1, dataByte2);
  // Serial.println(buffer);
#else
  Serial.write(statusByte);
  Serial.write(dataByte1);
  Serial.write(dataByte2);
#endif
}
int sensorPin = A0;
int sensorValue = 0;

void noteOn(int cmd, int pitch, int velocity) {
  sendMidi(cmd, 1, pitch, velocity);
  //Serial.write(cmd);
  //Serial.write(pitch);
  //Serial.write(velocity);
}
int senseMin = 25;
int senseMax = 120;

void loop() {
  sensorValue = analogRead(sensorPin);
  sensorValue = sensorValue >> 3;
  if (sensorValue < senseMin) senseMin = sensorValue;
  if (sensorValue > senseMax) senseMax = sensorValue;

  int note = map(sensorValue, senseMin, senseMax, 42, 90);
#ifdef SERIAL_DEBUGGING
  char sens[40];
  sprintf(sens, "sensor: %d, note: %d", sensorValue, note);
  Serial.println(sens);
#endif  // play notes from F#-0 (0x1E) to F#-5 (0x5A):
        //for (int note = 0x1E; note < 0x5A; note ++) {
  //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
  noteOn(0x90, note, 0x45);
  delay(100);
  //Note on channel 1 (0x90), some note value (note), silent velocity (0x00):
  noteOn(0x90, note, 0x00);
  delay(100);
  //}
}