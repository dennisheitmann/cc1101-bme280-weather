#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#define SCK 13
#define MISO 12
#define MOSI 11
#define CSN 10
#define GDO0 2
#define GDO2 9

Adafruit_BME280 bme; // I2C

unsigned long previousMillis = 0;
unsigned long interval = 1000;

struct dispvals_struct {
  int temp;
  int baro;
  int humi;
};

byte datasend[11];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  } else {
    Serial.println("BME280 initialized");
  }
  // CC1101
  ELECHOUSE_cc1101.setSpiPin(SCK, MISO, MOSI, CSN); //custom SPI pins. Set your own Spi Pins.Or to switch between multiple cc1101. Must be set before init and before changing the cc1101.
  Serial.println(F("setSpiPin"));
  ELECHOUSE_cc1101.Init();            // must be set to initialize the cc1101!
  Serial.println(F("Init"));
  ELECHOUSE_cc1101.setCCMode(1);      // set config for internal transmission mode.
  Serial.println(F("setCCMode(1)"));
  ELECHOUSE_cc1101.setModulation(1); // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
  Serial.println(F("setModulation(0)"));
  ELECHOUSE_cc1101.setMHZ(433.92); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
  Serial.println(F("setMHZ(433.92)"));
  ELECHOUSE_cc1101.setRxBW(203.125);       // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
  Serial.println(F("setRxBW(203.125)"));
  ELECHOUSE_cc1101.setSyncMode(2);  // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
  Serial.println(F("setSyncMode(2)"));
  ELECHOUSE_cc1101.setCrc(1);     // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.
  Serial.println(F("setCrc(1)"));
  Serial.println(F("CC1101 initialized"));
}

int readTemp() {
  // +30 K for "always" positive values
  int temp = ((bme.readTemperature() + 30.0F) * 100.0F);
  return temp;
}

int readBaro() {
  int baro = (bme.readPressure() * 0.1F);
  return baro;
}

int readHumi() {
  int humi = (bme.readHumidity() * 100.0F);
  return humi;
}

void printVals(dispvals_struct dvalues) {
  Serial.println(F("---------- Values ----------"));
  Serial.print(F("Temperature = "));
  Serial.print(((dvalues.temp / 100.0F) - 30.0F), 2);
  Serial.println(F(" *C"));
  Serial.print(F("Pressure = "));
  Serial.print(dvalues.baro / 10.F, 1);
  Serial.println(F(" mbar (abs)"));
  Serial.print(F("Humidity = "));
  Serial.print(dvalues.humi / 100.0F, 2);
  Serial.println(F(" % (rel)"));
  Serial.println();
}

dispvals_struct readAllValues() {
  dispvals_struct dvalues = {readTemp(), readBaro(), readHumi()};
  datasend[0] = (byte)atoi("D");
  datasend[1] = (byte)atoi("H");
  datasend[2] = (byte)(dvalues.temp / 100);
  datasend[3] = (byte)(dvalues.temp % 100);
  datasend[4] = (byte)(dvalues.baro / 100);
  datasend[5] = (byte)(dvalues.baro % 100);
  datasend[6] = (byte)(dvalues.humi / 100);
  datasend[7] = (byte)(dvalues.humi % 100);
  datasend[8] = (byte)atoi("D");
  datasend[9] = (byte)atoi("H");
  return dvalues;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - previousMillis >= interval) {
    dispvals_struct dvalues = readAllValues();
    printVals(dvalues);
    //    for (int i = 0; i < (sizeof(datasend) - 1); i = i + 2) {
    //      Serial.print(i);
    //      Serial.print(',');
    //      Serial.print(i + 1);
    //      Serial.print('=');
    //      Serial.print((int)((datasend[i] * 100) + datasend[i + 1]));
    //      Serial.print(':');
    //    }
    //    Serial.println();
    byte len = sizeof(datasend) - 1;
    ELECHOUSE_cc1101.SendData(datasend, 10, 100);
    previousMillis = millis();
  }
}
