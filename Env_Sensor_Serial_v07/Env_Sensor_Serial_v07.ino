
// Environmental Datalogger v1.7
// Load relevant libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
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
#define SEALEVELPRESSURE_HPA (1013.25)

RTC_PCF8523 rtc;                                          // Datalogger Real Time Clock
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // Env Sensor software SPI
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);            // Light Sensor software I2C

// for the data logging shield, we use digital pin 9 for the SD cs line
const int chipSelect = 9;

// Set logging file
File logfile;

// Setting up neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

// Throws error and halts program if there is a problem with writing or opening the SD card
void error(char *str) {
  Serial.print("error: ");
  Serial.println(str);
  while (1);
}

// Specify days of the week for RTC
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Create variable for month counting
unsigned char monthCount;

// Set up the feeds for the BME280 sensor (if IO compatible, not currently)
//AdafruitIO_Feed *temperatureFeed = io.feed("temperature");
//AdafruitIO_Feed *humidityFeed = io.feed("humidity");
//AdafruitIO_Feed *pressureFeed = io.feed("pressure");
//AdafruitIO_Feed *altitudeFeed = io.feed("altitude");

void setup() {
  // Put your setup code here, to run once:
  // Start the serial connection
  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Wait for serial monitor to open
  while (!Serial) {
    delay(1);  // for Leonardo/Micro/Zero
    Serial.println("Arduino Environmental Logger v1.7");
  }

  // Initialise neopixel and set to off
  SPI.begin();
  pixels.begin();
  pixels.clear();

  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to output, even if you don't use it:
  pinMode(10, OUTPUT);
  SD.begin(chipSelect);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

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

  Serial.print("Logging to: ");
  Serial.println(filename);

  // Default settings
  unsigned status;
  status = bme.begin();
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(5000);
    pixels.clear();
    while (1) delay(10);
  }

  // Initialise light sensor
  if (tsl.begin())
  {
    Serial.println(F("Found a TSL2591 sensor"));
  }
  else
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }

  // Display light sensor information
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);

  // Set light sensor gain and integration time (for outside conditions low gain and short integration times recommended)
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light) can be set to _MED (x25) or _HIGH (x428)
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light) can be set up to _600MS in 100ms intervals

  // Display the gain and integration time for reference sake
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch (gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));

  // Write data headers in log file
  logfile.println("Date/Time (YYYY:MM:DD HH:MM:SS), Temperature (*C), Pressure (Pa), Humidity (%), Infrared, Visible, Full Spectrum, Lux (lm/m^2)");
  logfile.flush();
  Serial.println("-- Start of Log --");
  Serial.println();
  Serial.println("Date/Time (YYYY:MM:DD HH:MM:SS), Temperature (*C), Pressure (Pa), Humidity (%), Infrared, Visible, Full Spectrum, Lux (lm/m^2)");

}

void loop() {
  // Put your main code here, to run repeatedly:
  DateTime now = rtc.now();
  delay(10000);
  printValues(now);

  //  // Write to new file every month
  //  if (now.month() != monthCount) {
  //    logfile.close();
  //    char filename[] = "LOGGER00.CSV";
  //    for (uint8_t i = 0; i < 100; i++) {
  //      filename[6] = i / 10 + '0';
  //      filename[7] = i % 10 + '0';
  //      if (!SD.exists(filename)) {
  //        // only open a new file if it doesn't exist
  //        logfile = SD.open(filename, FILE_WRITE);
  //        break;  // leave the loop!
  //      }
  //    }
  //    // Write data headers in log file
  //    logfile.println("Date/Time (YYYY:MM:DD HH:MM:SS), Temperature (*C), Pressure (Pa), Humidity (%)");
  //    logfile.flush();
  //
  //    // Collect the month
  //    monthCount = now.month();
  //  }
  //
  //  if (now.minute() == 00 && now.second() == 00) {
  //    printValues(now);
  //  }
  //
  //  if (now.minute() == 30 && now.second() == 00) {
  //    printValues(now);
  //  }
}

void printValues(DateTime now) {

  Serial.println("Reading Sensors...");

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

  // Read temperature from BME280
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println("*C");

  // Read pressure from BME280
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure());
  Serial.println("Pa");

  // Read approximate altitude from BME280
  Serial.print("Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println("m");

  // Read humidity from BME280
  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println("%");

  Serial.println();

  // Read light sensor
  Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
  Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
  Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
  Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 3);

  Serial.println();

  // Wait a second otherwise the loop will be called again
  delay(1000);
}
