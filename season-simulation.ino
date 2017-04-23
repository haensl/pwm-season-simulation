// uncomment to do a dry run
// #define DRY_RUN
#include <Process.h>
#include <Math.h>
#define COLON ":"

#ifdef DRY_RUN
#include <Console.h>
#endif

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
 * Wait this many milliseconds before initializing the bridge, to give the linux kernel time to boot.
 */
const unsigned int BOOT_DELAY = 5000;

boolean lunarCycleCompleted;
boolean moonlightShiningAtLastIteration;

void setup() {
  lunarCycleCompleted = true;
  moonlightShiningAtLastIteration = false;
  pinMode(PIN_MOONLIGHT, OUTPUT);
  analogWrite(PIN_MOONLIGHT, 0);
  delay(BOOT_DELAY);
  Bridge.begin(BAUT_BRIDGE);

#ifdef DRY_RUN
  Console.begin();
  while(!Console);
  Console.println("Starting dry run.");

  for (unsigned int dayOfYear = 1; dayOfYear < 357; dayOfYear++) {
    byte dayOfMonth = dayOfYear % DAYS_PER_MONTH;
    for (byte hour = 0; hour < HOURS_PER_DAY; hour++) {
      lunarCycleDryRun(dayOfYear, dayOfMonth, { hour, 30, 0 });
    }
  }
#endif
}

void loop() {
#ifdef DRY_RUN
  
#else
  analogWrite(PIN_MOONLIGHT, lunarCycle());
#endif
}

#ifdef DRY_RUN
byte lunarCycleDryRun(unsigned int dayOfYear, byte dayOfMonth, Time t) {
  Console.println("");
  Console.print(dayOfYear);
  Console.print(". day of year\n");
  Console.print(dayOfMonth);
  Console.print(". day of month\n");
  Console.print("\nlunar cycle completed ");
  Console.print(lunarCycleCompleted);
  Console.print("\nmoonlight shining at last iteration ");
  Console.print(moonlightShiningAtLastIteration);

  static byte hoursOfMoonlight;
  static byte hourOfMoonrise;
  static byte maximumMoonlightLuminosityOfDay;

  if (lunarCycleCompleted) {
    hoursOfMoonlight = getHoursOfMoonlight(dayOfYear);
    hourOfMoonrise = getHourOfMoonrise(dayOfYear);
    maximumMoonlightLuminosityOfDay = getMaximumMoonlightLuminosity(dayOfMonth);
  }

  Console.print("\nhours of moonlight ");
  Console.print(hoursOfMoonlight);
  Console.print("\nhour of moonrise ");
  Console.print(hourOfMoonrise);
  Console.print("\nmaximum moonlight luminosity of day ");
  Console.print(maximumMoonlightLuminosityOfDay);

  byte moonlightLuminosity = getMoonlightLuminosity(hoursOfMoonlight, hourOfMoonrise, maximumMoonlightLuminosityOfDay, t);
  Console.print("\nmoonlight luminosity\n");
  Console.print(t.hours);
  Console.print(":");
  Console.print(t.minutes);
  Console.print(" - ");
  Console.print(moonlightLuminosity);
  Console.print("\n ---");
  Console.flush();
  lunarCycleCompleted = moonlightShiningAtLastIteration && moonlightLuminosity <= 0;
  moonlightShiningAtLastIteration = moonlightLuminosity > 0;
  return moonlightLuminosity;
}
#endif

byte lunarCycle() {
  static unsigned int dayOfYear;
  static byte dayOfMonth;
  static byte hoursOfMoonlight;
  static byte hourOfMoonrise;
  static byte maximumMoonlightLuminosityOfDay;

  if (lunarCycleCompleted) {
    dayOfYear = getDayOfYear();
    dayOfMonth = getDayOfMonth();
    hoursOfMoonlight = getHoursOfMoonlight(dayOfYear);
    hourOfMoonrise = getHourOfMoonrise(dayOfYear);
    maximumMoonlightLuminosityOfDay = getMaximumMoonlightLuminosity(dayOfMonth);
  }

  byte moonlightLuminosity = getMoonlightLuminosity(hoursOfMoonlight, hourOfMoonrise, maximumMoonlightLuminosityOfDay, getTime());
  lunarCycleCompleted = moonlightShiningAtLastIteration && moonlightLuminosity <= 0;
  moonlightShiningAtLastIteration = moonlightLuminosity > 0;
  return moonlightLuminosity;
}

byte getHoursOfMoonlight(unsigned int dayOfYear) {
  return (byte)round((MAX_MOONLIGHT_HOURS_PER_DAY / M_PI) * asin(cos(M_PI * dayOfYear / DAYS_PER_MONTH + SKEW_START_OF_YEAR_MOONLIGHT_LUMINOSITY)) + (MAX_MOONLIGHT_HOURS_PER_DAY / 2.0f));
}

byte getHourOfMoonrise(unsigned int dayOfYear) {
  return (byte)round((HOURS_PER_DAY / M_PI) * asin(cos(M_PI * dayOfYear / DAYS_PER_MONTH + SKEW_MONTHLY_MOONLIGHT_LUMINOSITY)) + (HOURS_PER_DAY / 2.0f)) % HOURS_PER_DAY;
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
