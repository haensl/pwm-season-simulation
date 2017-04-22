#include <Process.h>
#include <Console.h>

Process date;
const byte PIN_MOONLIGHT = 6;

const byte LUMINOSITY_MAX = 16;
const byte LUMINOSITY_INCREMENT = 1;
const unsigned long MILLIS_PER_LUMINOSITY_STEP = (12L * 60L * 1000L);
const unsigned int BAUT_CONSOLE = 9600;
const unsigned long BAUT_BRIDGE = 115200L;
const unsigned int BOOT_DELAY = 5000;

const byte HOURS_OF_MOONLIGHT = 6;
const byte HOURS_START = 21;
const byte HOURS_END = (HOURS_START + HOURS_OF_MOONLIGHT) % 24;

byte LUMINOSITY_MODIFIER = 1;

void setup() {
  Console.begin();
  while(!Console);
  Console.println("Waiting for kernel to boot.");
  delay(BOOT_DELAY);
  Console.println("Bridging to AR9331.");
  Bridge.begin(BAUT_BRIDGE);
  pinMode(PIN_MOONLIGHT, OUTPUT);
  Console.println("Starting moonlight simulation");
}

void loop() {
  byte luminosity = 0;
  if (moonlightEnabled()) {
    luminosity = getLuminosity();
  }
  
  analogWrite(PIN_MOONLIGHT, luminosity);
}

boolean moonlightEnabled() {
  byte currentHour = getHours();
  return currentHour >= HOURS_START || currentHour <= HOURS_END;
}

byte getLuminosity() {
  static byte luminosity = 0;
  static unsigned long millisAtLastIteration = 0;
  unsigned long millisNow = millis();

  if (millisNow - millisAtLastIteration >= MILLIS_PER_LUMINOSITY_STEP) {
    luminosity += (LUMINOSITY_INCREMENT * LUMINOSITY_MODIFIER);
    Console.print("luminosity: ");
    Console.print(luminosity);
    Console.print("\n");
  }

  if (luminosity <= 0 || luminosity >= LUMINOSITY_MAX) {
    LUMINOSITY_MODIFIER  *= -1;
  }

  millisAtLastIteration = millisNow;
  return luminosity;
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

