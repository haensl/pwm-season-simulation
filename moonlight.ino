#include <Process.h>
#include <Math.h>
#define COLON ":"

typedef struct Time {
  byte hours;
  byte minutes;
  byte seconds;
} Time;

const byte PIN_MOONLIGHT = 6;
const byte LUMINOSITY_MAX_MOONLIGHT = 16; // maximum luminosity -- i.e. pwm at full moon
const byte MAX_MOONLIGHT_HOURS_PER_DAY = 12;
const byte DAYS_PER_MONTH = 30;
const byte HOURS_PER_DAY = 24;

/**
 * Skews adjust the moonlight luminosity based on location around the globe.
 */
const float SKEW_START_OF_YEAR_MOONLIGHT_LUMINOSITY = 4.2f;
const float SKEW_MONTHLY_MOONLIGHT_LUMINOSITY = 4.3f;

/**
 * Set correct baut rate depending on board.
 */
const unsigned long BAUT_BRIDGE = 115200L;

/**
 * Wait before initializing the bridge, to give the linux kernel time to boot.
 */
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
  analogWrite(PIN_MOONLIGHT, moonlight());
}

byte moonlight() {
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
  moonlightShiningAtLastIteration = moonlightLuminosity > 0;
  return moonlightLuminosity;
}

byte getHoursOfMoonlight(unsigned int dayOfYear) {
  return (byte)round((MAX_MOONLIGHT_HOURS_PER_DAY / M_PI) * asin(cos(M_PI * dayOfYear / DAYS_PER_MONTH + SKEW_START_OF_YEAR_MOONLIGHT_LUMINOSITY)) + (MAX_MOONLIGHT_HOURS_PER_DAY / 2.0f));
}

byte getHourOfMoonrise(unsigned int dayOfYear) {
  return (byte)round((HOURS_PER_DAY / M_PI) * asin(cos(M_PI * dayOfYear / DAYS_PER_MONTH + SKEW_MONTHLY_MOONLIGHT_LUMINOSITY)) + (HOURS_PER_DAY / 2.0f));
}

byte getMaximumMoonlightLuminosity(byte dayOfMonth) {
  return (byte)round(-(LUMINOSITY_MAX_MOONLIGHT / 2.0f) * (2.0f / M_PI) * asin(cos(M_PI * dayOfMonth / (DAYS_PER_MONTH / 2.0f))) + (LUMINOSITY_MAX_MOONLIGHT / 2.0f));
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

