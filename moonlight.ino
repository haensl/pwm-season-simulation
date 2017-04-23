#include <Process.h>
#include <Math.h>
#define COLON ":"

typedef struct Time {
  byte hours;
  byte minutes;
  byte seconds;
} Time;

const byte PIN_MOONLIGHT = 6;
const byte LUMINOSITY_MAX = 16; // maximum luminosity -- i.e. pwm at full moon

const unsigned long MILLIS_PER_LUMINOSITY_STEP = (12L * 60L * 1000L);
const unsigned long BAUT_BRIDGE = 115200L;
const unsigned int BAUT_SERIAL = 9600;
const unsigned int BOOT_DELAY = 5000;

boolean mooncycleCompleted;
boolean moonlightShiningAtLastIteration;

void setup() {
  mooncycleCompleted = true;
  moonlightShiningAtLastIteration = false;
  pinMode(PIN_MOONLIGHT, OUTPUT);
  analogWrite(PIN_MOONLIGHT, 0);
  delay(BOOT_DELAY);
  Bridge.begin(BAUT_BRIDGE);
}

void loop() {
  static unsigned int dayOfYear;
  static byte dayOfMonth;
  static byte hoursOfMoonlight;
  static byte hourOfMoonrise;
  static byte maximumMoonlightLuminosityOfDay;

  if (mooncycleCompleted) {
    dayOfYear = getDayOfYear();
    dayOfMonth = getDayOfMonth();
    hoursOfMoonlight = getHoursOfMoonlight(dayOfYear);
    hourOfMoonrise = getHourOfMoonrise(dayOfYear);
    maximumMoonlightLuminosityOfDay = getMaximumMoonlightLuminosity(dayOfMonth);
  }

  byte moonlightLuminosity = getMoonlightLuminosity(hoursOfMoonlight, hourOfMoonrise, maximumMoonlightLuminosityOfDay, getTime());
  mooncycleCompleted = moonlightShiningAtLastIteration && moonlightLuminosity <= 0;

  analogWrite(PIN_MOONLIGHT, moonlightLuminosity);
  moonlightShiningAtLastIteration = moonlightLuminosity > 0;
}

byte getHoursOfMoonlight(unsigned int dayOfYear) {
  return 6 * (2 / M_PI) * asin(cos(M_PI * dayOfYear / 30 + 4.2)) + 6;
}

byte getHourOfMoonrise(unsigned int dayOfYear) {
  return 12 * (2 / M_PI) * asin(cos(M_PI * dayOfYear / 30 + 4.3)) + 12;
}

byte getMaximumMoonlightLuminosity(byte dayOfMonth) {
  return -(LUMINOSITY_MAX / 2) * (2 / M_PI) * asin(cos(M_PI * dayOfMonth / 15)) + (LUMINOSITY_MAX / 2);
}

byte getMoonlightLuminosity(byte hoursOfMoonlight, byte hourOfMoonrise, byte maxLuminosity, Time t) {
  float currentMoonhour = (t.hours - hourOfMoonrise) + (t.minutes / 60.0f);
  float hourOfMaximumLuminosity = hoursOfMoonlight / 2.0f;

  if (currentMoonhour >= 0 && currentMoonhour <= hoursOfMoonlight) {
    return (byte)round((maxLuminosity / M_PI) * asin(cos(M_PI * ((currentMoonhour + hourOfMaximumLuminosity) / hourOfMaximumLuminosity))) + (maxLuminosity / 2.0f));
  }
  
  return 0;
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

