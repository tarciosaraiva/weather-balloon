#include <Adafruit_BME280.h>
#include <TimeLib.h>
#include <Arducam_Mega.h>
#include <GPSParser.h>
#include <LoRa.h>
#include <SD.h>
#include <SPI.h>

// constants
constexpr int CAM_BUFFER_SIZE = 255;
constexpr int CAMERA_CS = 6;
constexpr int SDCARD_CS = 4;

constexpr int BME_AWS_ADDR = 0x77;
constexpr long LORA_FREQ = 915E6;

// globals
unsigned long captureStart = 0;

Adafruit_BME280 bme;
Arducam_Mega* myCAM = nullptr;
File outFile;
GPSReader gps(Serial1);

time_t toEpoch(const GPSData &gps_data)
{
  if (gps_data.time.length() < 8 || gps_data.date.length() < 10)
    return 0;

  tmElements_t tm;
  tm.Hour   = gps_data.time.substring(0, 2).toInt();
  tm.Minute = gps_data.time.substring(3, 5).toInt();
  tm.Second = gps_data.time.substring(6, 8).toInt();
  tm.Day    = gps_data.date.substring(0, 2).toInt();
  tm.Month  = gps_data.date.substring(3, 5).toInt();
  tm.Year   = gps_data.date.substring(6, 10).toInt() - 1970;
  return makeTime(tm);
}

void buildFilename(const GPSData &gps_data, char *name)
{
  time_t t = toEpoch(gps_data) + (10UL * 3600UL);
  tmElements_t tm;
  breakTime(t, tm);
  sprintf(name, "pics/%02d%02d%02d.jpg", tm.Hour, tm.Minute, tm.Second);
}

void openImageFile(const GPSData &gps_data)
{
  char name[29] = {0};
  buildFilename(gps_data, name);
  outFile = SD.open(name, FILE_WRITE);
  if (!outFile)
  {
    Serial.println(F("Could not open file."));
    while (1)
      ;
  }
}

void logTelemetry(const String &msg)
{
  File logFile = SD.open("obs/data.txt", FILE_WRITE);
  if (!logFile)
  {
    Serial.println(F("Could not open telemetry file."));
    return;
  }
  logFile.println(msg);
  logFile.close();
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

void captureImage(const GPSData &gps_data)
{
  captureStart = millis();
  Serial.print(F("Image capture started..."));
  openImageFile(gps_data);
  myCAM->takePicture(CAM_IMAGE_MODE_WQXGA2, CAM_IMAGE_PIX_FMT_JPG);
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

void setupDirectories()
{
  if (SD.mkdir("pics")) {
    Serial.println(F("'pictures' folder setup."));
  }

  if (SD.mkdir("obs")) {
    Serial.println(F("'telemetry' folder setup."));
  }
}

void setupSD()
{
  while (!SD.begin(SDCARD_CS))
  {
    Serial.println(F("SD init failed."));
    delay(1000);
  }
  setupDirectories();
  Serial.println(F("SD Card ready."));
}

void setupCamera()
{
  setPin(CAMERA_CS);
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
  while (!LoRa.begin(LORA_FREQ))
  {
    Serial.println(F("LoRa init failed, retrying..."));
    delay(1000);
  }

  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(11);
  // LoRa.setSignalBandwidth(250E3);

  Serial.println(F("LoRa ready."));
}

String buildTelemetryMessage(const GPSData &gps_data)
{
  String msg = "TS:";
  msg += (uint32_t)toEpoch(gps_data);
  msg += ",LAT:" + String(gps_data.latitude, 6);
  msg += ",LON:" + String(gps_data.longitude, 6);
  msg += ",ALT:" + String(gps_data.altitude, 2);
  msg += ",TMP:" + String(bme.readTemperature());
  msg += ",HUM:" + String(bme.readHumidity());
  msg += ",PRS:" + String(bme.readPressure() / 100.0F);
  return msg;
}

void transmitData(const String &msg)
{
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
}

void setup()
{
  setupSerial();

  setPin(SDCARD_CS);

  setupLoRa();
  setupSD();

  myCAM = new Arducam_Mega(CAMERA_CS);
  delay(200);
  setupCamera();
  setupBME280();
}

void loop()
{
  GPSData gps_data = gps.get_data();
  String telemetry = buildTelemetryMessage(gps_data);
  logTelemetry(telemetry);

  if (gps_data.has_fix)
  {
    Serial.println(F("Fix acquired!"));
    captureImage(gps_data);
    transmitData(telemetry);
  } else {
    Serial.println(F("No fix yet, retrying..."));
  }

  delay(10000);
}
