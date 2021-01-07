
// Environmental Datalogger v1.0
// Load relevant libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Define Sensor ports
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

// set whether to print to console (On = 1, Off = 0)
#define ECHO_TO_SERIAL 1

// Define Sea Level Pressure
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
RTC_PCF8523 rtc;                                          // Datalogger Real Time Clock

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// Set logging file
File logfile;

// Throws error and halts program if there is a problem with writing or opening the SD card
void error(char *str) {
  Serial.print("error: ");
  Serial.println(str);
  while (1);
}

// Set delay between data points (sec) & SLP
unsigned long delayTime;

// Specify days of the week for RTC
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

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
    Serial.println("Arduino Environmental Logger");
  }

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

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
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
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  // For unsigned delay variable
  unsigned status;

  // Default settings
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
    while (1) delay(10);
  }

  Serial.println("-- Start of Log --");
  //delayTime = 1800000;
  delayTime = 60000*5; // 5 Minutes for testing
  Serial.println();

  // Write data headers in log file
  logfile.println("Date/Time (YYYY:MM:DD HH:MM:SS), Temperature (*C), Pressure (Pa), Humidity (%)");

}

void loop() {
  // Put your main code here, to run repeatedly:
  printValues();
  delay(delayTime);
}

void printValues() {

  Serial.println("Reading Sensors...");

  // Collect time stamp from RTC
  DateTime now = rtc.now();

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

#if ECHO_TO_SERIAL
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
#endif
}
