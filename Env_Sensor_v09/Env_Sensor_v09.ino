// Environmental Datalogger v0.9
// Load relevant libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SleepyDog.h>
#include "RTClib.h"
#include "Adafruit_TSL2591.h"

// Define Sensor ports
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

// Define Neopixel pin & number
#define NEOPIN 40
#define NUMPIXELS 1

// Define Sea Level Pressure
//#define SEALEVELPRESSURE_HPA (1013.25)

RTC_PCF8523 rtc;                                          // Datalogger Real Time Clock
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);            // Light Sensor software I2C

// for the data logging shield, we use digital pin 9 for the SD cs line
const int chipSelect = 9;

// Set logging file
File logfile;

// Setting up neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

// Throws error and halts program if there is a problem with writing or opening the SD card
void error(char *str) {
  while (1);
}

// Specify days of the week for RTC
// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Create variable for month counting
unsigned char monthCount;

// Set up the feeds for the BME280 sensor (if IO compatible, not currently)
//AdafruitIO_Feed *temperatureFeed = io.feed("temperature");
//AdafruitIO_Feed *humidityFeed = io.feed("humidity");
//AdafruitIO_Feed *pressureFeed = io.feed("pressure");
//AdafruitIO_Feed *altitudeFeed = io.feed("altitude");

void setup() {
  // Put your setup code here, to run once:
  // Initialise neopixel and set to off
  SPI.begin();
  pixels.begin();
  pixels.clear();

  if (! rtc.begin()) {
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
    while (1);
  }

  if (! rtc.initialized()) {
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 255, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // initialize the SD card
  // make sure that the default chip select pin is set to output, even if you don't use it:
  pinMode(10, OUTPUT);
  SD.begin(chipSelect);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
    // don't do anything more:
    return;
  }

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (!SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    error("couldnt create file");
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
  }

  //Obtain the current month for monthCount
  DateTime now = rtc.now();
  monthCount = now.month();

  // Default settings
  unsigned status;
  status = bme.begin();
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status) {
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
    while (1) delay(10);
  }

  // Initialise light sensor
  status = tsl.begin();

  if (!status) {
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
    while (1) delay(10);
  }

  // Set light sensor gain and integration time (for outside conditions low gain and short integration times recommended)
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light) can be set to _MED (x25) or _HIGH (x428)
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light) can be set up to _600MS in 100ms intervals

  // Make LED available for showing sleep mode
  pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, HIGH); // Show we're awake, not sure if needed in set-up

  // Write data headers in log file
  logfile.println("Date/Time (YYYY:MM:DD HH:MM:SS), Temperature (*C), Pressure (Pa), Humidity (%), Infrared, Visible, Full Spectrum, Lux (lm/m^2)");
  logfile.flush();

}

void loop() {
  // Put your main code here, to run repeatedly:
  DateTime now = rtc.now();

  // Write to new file every month
  if (now.month() != monthCount) {
    logfile.close();
    char filename[] = "LOGGER00.CSV";
    for (uint8_t i = 0; i < 100; i++) {
      filename[6] = i / 10 + '0';
      filename[7] = i % 10 + '0';
      if (!SD.exists(filename)) {
        // only open a new file if it doesn't exist
        logfile = SD.open(filename, FILE_WRITE);
        break;  // leave the loop!
      }
    }
    // Write data headers in log file
    logfile.println("Date/Time (YYYY:MM:DD HH:MM:SS), Temperature (*C), Pressure (Pa), Humidity (%), Infrared, Visible, Full Spectrum, Lux (lm/m^2)");
    logfile.flush();

    // Collect the month
    monthCount = now.month();
  }

  if (now.minute() == 00 && now.second() == 00) {
    printValues(now);
  }

  if (now.minute() == 30 && now.second() == 00) {
    printValues(now);
  }
}

void printValues(DateTime now) {

  logfile.print(now.year(), DEC);
  logfile.print('/');
  logfile.print(now.month(), DEC);
  logfile.print('/');
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(':');
  logfile.print(now.minute(), DEC);
  logfile.print(':');
  logfile.print(now.second(), DEC);
  logfile.print(", ");

  // Log sensor data
  logfile.print(bme.readTemperature());
  logfile.print(", ");
  logfile.print(bme.readPressure());
  logfile.print(", ");
  logfile.print(bme.readHumidity());
  logfile.print(", ");

  // Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;

  logfile.print(ir);
  logfile.print(", ");
  logfile.print(full - ir);
  logfile.print(", ");
  logfile.print(full);
  logfile.print(", ");
  logfile.print(tsl.calculateLux(full, ir), 3);

  logfile.println();

  // Save to file
  logfile.flush();

  // Wait a second ensure SD has done its business
  delay(3000);

  // Show going to sleep
  digitalWrite(LED_BUILTIN, LOW); 

  // Sleep for current set period (90*16 seconds ~25 mins + 3 second delay*90 ~ 4.5 mins)
  for(int i=0; i<110; i++) {
  int sleepMS = Watchdog.sleep();
  }

    // Show awake
  digitalWrite(LED_BUILTIN, HIGH); 
  
}
