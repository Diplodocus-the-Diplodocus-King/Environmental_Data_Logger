// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include <Adafruit_NeoPixel.h>
#include "RTClib.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        40
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {

  while (!Serial) {
    delay(1);  // for Leonardo/Micro/Zero
  }

  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(150, 0, 0));
  pixels.show();
  delay(5000);
  pixels.clear();
}

void loop () {
  DateTime now = rtc.now();

  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);

  Serial.println();

  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, 150, 0));
  pixels.show();
  delay(5000);
  pixels.clear();

  // Example test for seconds and days since 1st Jan 1970 (unix time)
  //    Serial.print(" since midnight 1/1/1970 = ");
  //    Serial.print(now.unixtime());
  //    Serial.print("s = ");
  //    Serial.print(now.unixtime() / 86400L);
  //    Serial.println("d");

  // calculate a date which is 7 days, 12 hours and 30 seconds into the future
  // DateTime future (now + TimeSpan(7,12,30,6));

  //    Serial.print(" now + 7d + 12h + 30m + 6s: ");
  //    Serial.print(future.year(), DEC);
  //    Serial.print('/');
  //    Serial.print(future.month(), DEC);
  //    Serial.print('/');
  //    Serial.print(future.day(), DEC);
  //    Serial.print(' ');
  //    Serial.print(future.hour(), DEC);
  //    Serial.print(':');
  //    Serial.print(future.minute(), DEC);
  //    Serial.print(':');
  //    Serial.print(future.second(), DEC);
  //    Serial.println();

  //    Serial.println();
  delay(3000);
}
