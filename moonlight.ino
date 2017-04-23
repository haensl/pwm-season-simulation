#include <Process.h>
#include <Math.h>
#define DEBUG true
#define COLON ":"

typedef struct Time {
  byte hours;
  byte minutes;
  byte seconds;
} Time;

Process date;
const byte PIN_MOONLIGHT = 6;
const byte LUMINOSITY_MAX = 16; // maximum luminosity -- i.e. pwm at full moon

const unsigned long MILLIS_PER_LUMINOSITY_STEP = (12L * 60L * 1000L);
const unsigned long BAUT_BRIDGE = 115200L;
const unsigned int BAUT_SERIAL = 9600;
const unsigned int BOOT_DELAY = 5000;

byte LUMINOSITY_MODIFIER = 1;
byte luminosity;
unsigned long millisSinceLastLuminosityChange;

void setup() {
  pinMode(PIN_MOONLIGHT, OUTPUT);
  analogWrite(PIN_MOONLIGHT, 0);
  if (DEBUG) {
    Serial.begin(BAUT_SERIAL);
    while(!Serial);
    Serial.println("Waiting for kernel to boot.");
  }
  delay(BOOT_DELAY);
  Bridge.begin(BAUT_BRIDGE);
  luminosity = 0;
  millisSinceLastLuminosityChange = 0;
  if (DEBUG) {
    Serial.println("Starting moonlight simulation");
  }

  for (int i = 1; i < 366; i++) {
    byte hoursOfMoonlight = getMoonlightHours(i);
    byte moonriseHour = getMoonriseHour(i);

    Serial.print("Day ");
    Serial.print(i);
    Serial.print("\n");
    Serial.print("Hours of moonlight: ");
    Serial.print(hoursOfMoonlight);
    Serial.print("\n");
    Serial.print("Moon is rising at: ");
    Serial.print(moonriseHour);
    Serial.print("\n");
    Serial.print("---\n");
  }

  for (int j = 1; j < 31; j++) {
    byte luminosityMax = getMaximumLuminosity(j);
    Serial.print("Day ");
    Serial.print(j);
    Serial.print("\n");
    Serial.print("Maximum moonlight luminosity: ");
    Serial.print(luminosityMax);
    Serial.print("\n");
    Serial.print("---\n");
  }
}

void loop() {
/**
  Time now = getTime();
  unsigned int dayOfYear = getDayOfYear();
  byte dayOfMonth = getDayOfMonth();
  byte hoursOfMoonlight = getMoonlightHours(dayOfYear);
  byte moonriseHour = getMoonriseHour(dayOfYear);
  byte moonsetHour = (moonriseHour + hoursOfMoonlight) % 24;
  byte luminosityMax = getMaximumLuminosity(16);

  if (DEBUG) {
    Serial.print("Current time: ");
    Serial.print(now.hours);
    Serial.print(COLON);
    Serial.print(now.minutes);
    Serial.print(COLON);
    Serial.print(now.seconds);
    Serial.print("\n");
    Serial.print("Day of year: ");
    Serial.print(dayOfYear);
    Serial.print("\n");
    Serial.print("Hours of moonlight: ");
    Serial.print(hoursOfMoonlight);
    Serial.print("\n");
    Serial.print("Moon is rising at: ");
    Serial.print(moonriseHour);
    Serial.print("\n");
    Serial.print("Moon is setting at: ");
    Serial.print(moonsetHour);
    Serial.print("\n");
    Serial.print("Day of month: ");
    Serial.print(dayOfMonth);
    Serial.print("\n");
    Serial.print("Maximum moonlight luminosity: ");
    Serial.print(luminosityMax);
    Serial.print("\n");
  }
  **/
  
  /**
  if (now.hours >= moonriseHour || now.hours <= moonsetHour) {
    luminosity = getLuminosity(luminosity);
  } else {
    luminosity = 0;
    millisSinceLastLuminosityChange = 0;
  }

  analogWrite(PIN_MOONLIGHT, luminosity);
  */
}

byte getMoonlightHours(unsigned int dayOfYear) {
  return 6 * (2 / M_PI) * asin(cos(M_PI * dayOfYear / 30 + 4.2)) + 6;
}

byte getMoonriseHour(unsigned int dayOfYear) {
  return 12 * (2 / M_PI) * asin(cos(M_PI * dayOfYear / 30 + 4.3)) + 12;
}

byte getMaximumLuminosity(byte dayOfMonth) {
  return -(LUMINOSITY_MAX / 2) * (2 / M_PI) * asin(cos(M_PI * dayOfMonth / 15)) + (LUMINOSITY_MAX / 2);
}

byte getLuminosity(byte currentLuminosity) {
  static unsigned long millisAtLastIteration = 0;
  unsigned long millisNow = millis();
  millisSinceLastLuminosityChange += (millisNow - millisAtLastIteration);
  byte lum = currentLuminosity;

  if (DEBUG) {
    Serial.print("millisSinceLastLuminosityChange: ");
    Serial.print(millisSinceLastLuminosityChange);
    Serial.print("\n");
  }

  if (millisSinceLastLuminosityChange >= MILLIS_PER_LUMINOSITY_STEP) {
    lum += (LUMINOSITY_INCREMENT * LUMINOSITY_MODIFIER);
    millisSinceLastLuminosityChange = 0;
    if (DEBUG) {
      Serial.print("luminosity: ");
      Serial.print(lum);
      Serial.print("\n");
    }
  }

  if (lum <= 0 || lum >= LUMINOSITY_MAX) {
    LUMINOSITY_MODIFIER  *= -1;
  }

  millisAtLastIteration = millisNow;
  return lum;
}

Time getTime() {
  Time t;

  Process date;
  date.begin("/bin/date");
  date.addParameter("+%T");
  date.run();

  String timeString;

  while (date.available() > 0) {
    timeString = date.readString();
  }

  byte firstColon = timeString.indexOf(COLON);
  byte secondColon = timeString.lastIndexOf(COLON);

  t = {
    (byte)timeString.substring(0, firstColon).toInt(),
    (byte)timeString.substring(firstColon + 1, secondColon).toInt(),
    (byte)timeString.substring(secondColon + 1).toInt()
  };

  return t;
}

unsigned int getDayOfYear() {
  Process date;
  date.begin("/bin/date");
  date.addParameter("+%j");
  date.run();
  String doy;

  while (date.available() > 0) {
    doy = date.readString();
  }

  return doy.toInt();
}

byte getDayOfMonth() {
  Process date;
  date.begin("/bin/date");
  date.addParameter("+%d");
  date.run();
  String dom;

  while (date.available() > 0) {
    dom = date.readString();
  }

  return (byte)dom.toInt();
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

