#include <Arduino.h>
#include <SD.h>
#include <Arducam_Mega.h>
#include <Adafruit_BME280.h>
// #include <SoftwareSerial.h>
// #include <TinyGPS++.h>

// constants
constexpr int CAM_BUFFER_SIZE = 255;
constexpr int GPS_TX_PIN = 2;
constexpr int GPS_RX_PIN = 3;
constexpr int CAMERA_PIN = 7;
constexpr int SD_CARD_PIN = 9;
constexpr int BME_AWS_ADDR = 0x77;
// constexpr int GPS_LED = LED_BUILTIN;

// globals
unsigned long start = 0;
unsigned long captureStart = 0;

Adafruit_BME280 bme;
Arducam_Mega myCAM(CAMERA_PIN);
File outFile;
// TinyGPSPlus gps;
// SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);

void openImageFile()
{
  char name[16] = {0};
  sprintf(name, "%lu.jpg", millis() - start);
  outFile = SD.open(name, FILE_WRITE);
  if (!outFile)
  {
    Serial.println(F("Could not open file."));
    while (1)
      ;
  }
}

void saveImage()
{
  uint8_t buffer[CAM_BUFFER_SIZE] = {0};
  uint32_t remaining = myCAM.getReceivedLength();
  while (remaining > 0)
  {
    uint8_t toRead = remaining > CAM_BUFFER_SIZE ? CAM_BUFFER_SIZE : (uint8_t)remaining;
    myCAM.readBuff(buffer, toRead);
    outFile.write(buffer, toRead);
    remaining -= toRead;
  }
  outFile.close();
  Serial.print(F("... and finished. Took: "));
  Serial.print(millis() - captureStart);
  Serial.println(F("ms."));
}

void captureImage()
{
  captureStart = millis();
  Serial.print(F("Image capture started..."));
  openImageFile();
  myCAM.takePicture(CAM_IMAGE_MODE_WQXGA2, CAM_IMAGE_PIX_FMT_JPG);
  saveImage();
}

void setupSerial()
{
  Serial.begin(9600);
  while(!Serial);
  Serial.println(F("Serial ready."));
}

void setupSD()
{
  pinMode(SD_CARD_PIN, OUTPUT);
  digitalWrite(SD_CARD_PIN, HIGH);

  while (!SD.begin(SD_CARD_PIN))
  {
    Serial.println(F("SD Card could not be initialised."));
    delay(1000);
  }
  Serial.println(F("SD Card ready."));
}

void setupCamera()
{
  myCAM.begin();
  Serial.println(F("Camera ready."));
}

void setupBME280()
{
  if (!bme.begin(BME_AWS_ADDR))
  {
    Serial.println("Could not find BME280!");
    while (1);
  }
  else
  {
    Serial.println(F("Atmospheric sensor ready."));
  }
}

void setup()
{
  start = millis();

  pinMode(CAMERA_PIN, OUTPUT);
  digitalWrite(CAMERA_PIN, HIGH);

  setupSerial();
  setupSD();
  setupCamera();
  setupBME280();
}

// void displayInfo()
// {
//   Serial.print(F("Location: "));
//   if (gps.location.isValid())
//   {
//     Serial.print(gps.location.lat(), 6);
//     Serial.print(F(","));
//     Serial.print(gps.location.lng(), 6);
//   }
//   else
//   {
//     Serial.print(F("INVALID"));
//   }

//   Serial.print(F("  Date/Time: "));
//   if (gps.date.isValid())
//   {
//     Serial.print(gps.date.month());
//     Serial.print(F("/"));
//     Serial.print(gps.date.day());
//     Serial.print(F("/"));
//     Serial.print(gps.date.year());
//   }
//   else
//   {
//     Serial.print(F("INVALID"));
//   }

//   Serial.print(F(" "));
//   if (gps.time.isValid())
//   {
//     if (gps.time.hour() < 10) Serial.print(F("0"));
//     Serial.print(gps.time.hour());
//     Serial.print(F(":"));
//     if (gps.time.minute() < 10) Serial.print(F("0"));
//     Serial.print(gps.time.minute());
//     Serial.print(F(":"));
//     if (gps.time.second() < 10) Serial.print(F("0"));
//     Serial.print(gps.time.second());
//     Serial.print(F("."));
//     if (gps.time.centisecond() < 10) Serial.print(F("0"));
//     Serial.print(gps.time.centisecond());
//   }
//   else
//   {
//     Serial.print(F("INVALID"));
//   }

//   Serial.println();
// }

// void retrieveLocation() {
//   // This sketch displays information every time a new sentence is correctly encoded.
//   while (ss.available() > 0) {
//     if (gps.encode(ss.read()))
//       displayInfo();
//   }

//   if (millis() > 5000 && gps.charsProcessed() < 10)
//   {
//     Serial.println(F("No GPS detected: check wiring."));
//     while(true);
//   }
// }

void loop()
{
  // retrieveLocation();
  captureImage();

  Serial.println(F("------------------------------"));
  Serial.print(F("Temp: "));
  Serial.println(bme.readTemperature());
  Serial.print(F("Humidity: "));
  Serial.println(bme.readHumidity());
  Serial.print(F("Pressure: "));
  Serial.println(bme.readPressure() / 100.0F);
  Serial.println(F("------------------------------"));

  delay(10000);
}
