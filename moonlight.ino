#include <Process.h>
#include <Console.h>

#define DEBUG true

Process date;
const byte PIN_MOONLIGHT = 6;

const byte LUMINOSITY_MAX = 16;
const byte LUMINOSITY_INCREMENT = 1;
const unsigned long MILLIS_PER_LUMINOSITY_STEP = (12L * 60L * 1000L);
const unsigned long BAUT_BRIDGE = 115200L;
const unsigned int BAUT_SERIAL = 9600;
const unsigned int BOOT_DELAY = 5000;

const byte HOURS_OF_MOONLIGHT = 6;
const byte HOURS_START = 21;
const byte HOURS_END = (HOURS_START + HOURS_OF_MOONLIGHT) % 24;

byte LUMINOSITY_MODIFIER = 1;
byte luminosity;
unsigned long millisSinceLastLuminosityChange;

void setup() {
  if (DEBUG) {
    Serial.begin(BAUT_SERIAL);
    while(!Serial);
    Serial.println("Waiting for kernel to boot.");
  }
  delay(BOOT_DELAY);
  Bridge.begin(BAUT_BRIDGE);
  pinMode(PIN_MOONLIGHT, OUTPUT);
  luminosity = 0;
  millisSinceLastLuminosityChange = 0;
  if (DEBUG) {
    Serial.println("Starting moonlight simulation");
  }
}

void loop() {
  byte currentHour = getHours();
  if (DEBUG) {
    Serial.print("H: ");
    Serial.print(currentHour);
    Serial.print("\n");
  }

  if (currentHour >= HOURS_START || currentHour <= HOURS_END) {
    luminosity = getLuminosity(luminosity);
  } else {
    luminosity = 0;
    millisSinceLastLuminosityChange = 0;
  }

  analogWrite(PIN_MOONLIGHT, luminosity);
}

byte getLuminosity(byte currentLuminosity) {
  static unsigned long millisAtLastIteration = 0;
  unsigned long millisNow = millis();
  millisSinceLastLuminosityChange += (millisNow - millisAtLastIteration);
  byte lum = currentLuminosity;

  if (DEBUG) {
    Serial.print("millisNow - millisAtLastIteration ");
    Serial.print((millisNow - millisAtLastIteration));
    Serial.print("\n");
  }

  if (millisSinceLastLuminosityChange >= MILLIS_PER_LUMINOSITY_STEP) {
    lum += (LUMINOSITY_INCREMENT * LUMINOSITY_MODIFIER);
    millisSinceLastLuminosityChange = 0;
    if (DEBUG) {
      Serial.print("luminosity: ");
      Serial.print(luminosity);
      Serial.print("\n");
    }
  }

  if (lum <= 0 || lum >= LUMINOSITY_MAX) {
    LUMINOSITY_MODIFIER  *= -1;
  }

  millisAtLastIteration = millisNow;
  return lum;
}

byte getHours() {
  date.begin("/bin/date");
  date.addParameter("+%H");
  date.run();
  byte hours = 0;

  while(date.available() > 0) {
    hours = date.parseInt();
  }

  return hours;
}

/**
unsigned long getTimestamp() {
  date.begin("/bin/date");
  date.addParameter("+%s");
  date.run();
  unsigned long timestamp = 0;

  while(date.available() > 0) {
    timestamp = date.parseInt();
  }

  return timestamp;
}*/

