#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#define SCK 13
#define MISO 12
#define MOSI 11
#define CSN 10
#define GDO0 2
#define GDO2 9

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

byte buffer[62] = {0};

struct dispvals {
  float temp;
  float baro;
  float humi;
  int lqi;
  int rssi;
};

int gross_klein = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    asm volatile("jmp 0 \n");  // jump to reset
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000);

  // Clear the buffer
  display.clearDisplay();
  for (int16_t i = 0; i < max(display.width(), display.height()) / 2; i += 2) {
    display.drawCircle(display.width() / 2, display.height() / 2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(1000);

  Serial.println(F("CC1101 Receive"));
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

  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.println("CC1101");
  display.println("RxMode");
  display.display();
}

void printVals(dispvals dvalues) {
  Serial.print(F("Temperature = "));
  Serial.print(dvalues.temp, 2);
  Serial.println(F(" *C"));
  Serial.print(F("Pressure = "));
  Serial.print(dvalues.baro, 1);
  Serial.println(F(" mbar (abs)"));
  Serial.print(F("Humidity = "));
  Serial.print(dvalues.humi, 2);
  Serial.println(F(" % (rel)"));
  Serial.print(F("LQI = "));
  Serial.print(dvalues.lqi);
  Serial.println(F(" LQI"));
  Serial.print(F("RSSI = "));
  Serial.print(dvalues.rssi);
  Serial.println(F(" RSSI"));
}

void printOnDisplay(dispvals dvalues) {
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.print(dvalues.temp);
  display.println(F(" *C"));
  display.print(dvalues.baro);
  display.println(F(" mbar (abs)"));
  display.print(dvalues.humi);
  display.println(F(" % (rel)"));
  display.print(dvalues.lqi);
  display.println(F(" LQI"));
  display.display();
}

void printTempOnDisplay(dispvals dvalues) {
  display.clearDisplay();
  display.setTextSize(3);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.print(dvalues.temp);
  display.println(F(" *C"));
  display.display();
}

void loop() {
  if (ELECHOUSE_cc1101.CheckRxFifo(250)) {
    if (ELECHOUSE_cc1101.CheckCRC()) {    //CRC Check. If "setCrc(false)" crc returns always OK!
      int len = ELECHOUSE_cc1101.ReceiveData(buffer);
      buffer[len] = '\0';
      //    for (int i = 0; i < (sizeof(buffer) - 1); i = i + 2) {
      //      Serial.print(i);
      //      Serial.print(',');
      //      Serial.print(i + 1);
      //      Serial.print('=');
      //      Serial.print((int)((buffer[i] * 100) + buffer[i + 1]));
      //      Serial.print(':');
      //    }
      if (buffer[0] == (byte)atoi("D")) {
        if (buffer[1] == (byte)atoi("H")) {
          if (buffer[8] == (byte)atoi("D")) {
            if (buffer[9] == (byte)atoi("H")) {
              float temp = ((buffer[2] * 100) + buffer[3]);
              float baro = ((buffer[4] * 100) + buffer[5]);
              float humi = ((buffer[6] * 100) + buffer[7]);
              Serial.println(F("---------- Values ----------"));
              int rssi = ELECHOUSE_cc1101.getRssi();
              int lqi = ELECHOUSE_cc1101.getLqi();
              dispvals dvalues = {((temp / 100.0F) - 30.0F), (baro / 10.0F), (humi / 100.0F), lqi, rssi};
              printVals(dvalues);
              Serial.println();
              if (gross_klein < 1) {
                printOnDisplay(dvalues);
                gross_klein = 10;
              } else {
                printTempOnDisplay(dvalues);
                gross_klein--;
              }
            }
          }
        }
      }
    }
    // ELECHOUSE_cc1101.SetRx();
  }
}
