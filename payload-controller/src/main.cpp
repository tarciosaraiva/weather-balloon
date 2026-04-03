#include <Adafruit_BME280.h>
#include <Arducam_Mega.h>
#include <Arduino.h>
#include <GPSParser.h>
#include <LoRa.h>
#include <SdFat.h>
#include <SdCard/SdSpiCard/SpiDriver/SdSpiSoftDriver.h>

// constants
constexpr int CAM_BUFFER_SIZE = 255;
constexpr int CAMERA_CS = 7;
constexpr int SDCARD_CS = 4;
constexpr int SD_MOSI_PIN = 5;
constexpr int SD_MISO_PIN = 6;
constexpr int SD_SCK_PIN = 8;

constexpr int BME_AWS_ADDR = 0x77;
constexpr int LORA_SS_PIN = 10;
constexpr int LORA_RESET_PIN = 9;
constexpr int LORA_DIO0_PIN = 2;
constexpr long LORA_FREQ = 915E6;

SoftSpiDriver<SD_MISO_PIN, SD_MOSI_PIN, SD_SCK_PIN> softSpi;
SdFat32 sd;
#define SD_CONFIG SdSpiConfig(SDCARD_CS, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)

// globals
unsigned long start = 0;
unsigned long captureStart = 0;

Adafruit_BME280 bme;
Arducam_Mega* myCAM = nullptr;
File32 outFile;
GPSReader gps(Serial1);

void openImageFile()
{
  char name[16] = {0};
  sprintf(name, "%lu.jpg", millis() - start);
  outFile = sd.open(name, O_WRONLY | O_CREAT | O_TRUNC);
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
  uint32_t remaining = myCAM->getReceivedLength();
  while (remaining > 0)
  {
    uint8_t toRead = remaining > CAM_BUFFER_SIZE ? CAM_BUFFER_SIZE : (uint8_t)remaining;
    myCAM->readBuff(buffer, toRead);
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
  myCAM->takePicture(CAM_IMAGE_MODE_FHD, CAM_IMAGE_PIX_FMT_JPG);
  saveImage();
}

void setPin(int pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void setupSerial()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial)
    ;
  Serial.println(F("Serial ready."));
}

void setupSD()
{
  while (!sd.begin(SD_CONFIG))
  {
    Serial.println(F("SD init failed."));
    delay(1000);
  }
  Serial.println(F("SD Card ready."));
}

void setupCamera()
{
  while (myCAM->begin() != CAM_ERR_SUCCESS)
  {
    Serial.println(F("Camera could not be initialised."));
    delay(1000);
  }
  Serial.println(F("Camera ready."));
}

void setupBME280()
{
  if (!bme.begin(BME_AWS_ADDR))
  {
    Serial.println("Could not find BME280!");
    while (1)
      ;
  }
  else
  {
    Serial.println(F("Atmospheric sensor ready."));
  }
}

void setupLoRa()
{
  LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
  while (!LoRa.begin(LORA_FREQ))
  {
    Serial.println(F("LoRa init failed, retrying..."));
    delay(1000);
  }

  // LoRa.setTxPower(20);
  // LoRa.setSpreadingFactor(11);
  // LoRa.setSignalBandwidth(250E3);

  Serial.println(F("LoRa ready."));
}

void transmitData(const GPSData &gps_data)
{
  Serial.println(F("transmiting data"));
  LoRa.beginPacket();
  LoRa.print("LAT:");
  LoRa.print(gps_data.latitude, 6);
  LoRa.print(",LON:");
  LoRa.print(gps_data.longitude, 6);
  LoRa.print(",ALT:");
  LoRa.print(gps_data.altitude, 2);
  LoRa.print(",TMP:");
  LoRa.print(bme.readTemperature());
  LoRa.print(",HUM:");
  LoRa.print(bme.readHumidity());
  LoRa.print(",PRS:");
  LoRa.print(bme.readPressure() / 100.0F);
  LoRa.endPacket();
}

void setup()
{
  start = millis();

  setupSerial();

  setPin(CAMERA_CS);
  setPin(SDCARD_CS);
  // setPin(LORA_SS_PIN);

  // setupLoRa();
  setupSD();

  myCAM = new Arducam_Mega(CAMERA_CS);
  delay(200);
  setupCamera();
  setupBME280();
}

void loop()
{
  captureImage();

  // Get the GPS data (automatically updates)
  GPSData gps_data = gps.get_data();

  // Print the GPS data
  Serial.print("Fix: ");
  Serial.print(gps_data.has_fix ? "Yes" : "No");

  Serial.print(", Latitude: ");
  Serial.print(gps_data.latitude, 6);

  Serial.print(", Longitude: ");
  Serial.print(gps_data.longitude, 6);

  Serial.print(", Altitude: ");
  Serial.print(gps_data.altitude, 6);

  // Additional data
  Serial.print(", Satellites: ");
  Serial.print(gps_data.satellites);

  Serial.print(", Time: ");
  Serial.print(gps_data.time);

  Serial.print(", Date: ");
  Serial.println(gps_data.date);

  Serial.println(F("------------------------------"));
  Serial.print(F("Temp: "));
  Serial.println(bme.readTemperature());
  Serial.print(F("Humidity: "));
  Serial.println(bme.readHumidity());
  Serial.print(F("Pressure: "));
  Serial.println(bme.readPressure() / 100.0F);
  Serial.println(F("------------------------------"));

  // transmitData(gps_data);

  delay(10000);
}
