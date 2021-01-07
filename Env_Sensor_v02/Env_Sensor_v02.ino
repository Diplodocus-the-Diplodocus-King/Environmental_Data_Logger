
// Environmental Datalogger v1.0
// Load relevant libraries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "RTClib.h"

// Define Sensor ports
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

// Define Sea Level Pressure
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
RTC_PCF8523 rtc;                                          // Datalogger Real Time Clock

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
while (!Serial){
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

unsigned status;

// Default settings
status = bme.begin();  
// You can also pass in a Wire library object like &Wire2
// status = bme.begin(0x76, &Wire2)
if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
        }
    
    Serial.println("-- Start of Log --");
    delayTime = 1800000;

    Serial.println();

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
}
