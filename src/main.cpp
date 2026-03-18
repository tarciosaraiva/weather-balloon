#include <Arducam_Mega.h>
#include <Arduino.h>
#include <ezLED.h>
#include <SD.h>

// constants
constexpr int CAM_BUFFER_SIZE = 255;
constexpr int CAMERA_PIN      = 7;
constexpr int RED_LED_PIN     = 9;
constexpr int SD_CARD_PIN     = 10;

// globals
unsigned long start        = 0;
unsigned long captureStart = 0;

ezLED        led(RED_LED_PIN);
Arducam_Mega myCAM(CAMERA_PIN);
File         outFile;

void openImageFile()
{
  char name[16] = {0};
  sprintf(name, "%10lu.jpg", millis() - start);
  outFile = SD.open(name, FILE_WRITE);
  if (!outFile)
  {
    Serial.println(F("Could not open file."));
    while (1);
  }
}

void saveImage()
{
  uint8_t  buffer[CAM_BUFFER_SIZE] = {0};
  uint32_t remaining               = myCAM.getReceivedLength();
  while (remaining > 0)
  {
    led.loop();
    uint8_t toRead = remaining > CAM_BUFFER_SIZE ? CAM_BUFFER_SIZE : (uint8_t)remaining;
    myCAM.readBuff(buffer, toRead);
    outFile.write(buffer, toRead);
    remaining -= toRead;
  }
  outFile.close();
  led.turnOFF();
  Serial.print(F("... and finished. Took: "));
  Serial.print(millis() - captureStart);
  Serial.println(F("ms."));
}

void captureImage()
{
  captureStart = millis();

  led.turnON();
  led.blink(50, 50);

  openImageFile();
  Serial.print(F("Image capture started..."));
  myCAM.takePicture(CAM_IMAGE_MODE_FHD, CAM_IMAGE_PIX_FMT_JPG);
  saveImage();
}

void setupSerial()
{
  Serial.begin(9600);
}

void setupSD()
{
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

void setup()
{
  start = millis();

  pinMode(CAMERA_PIN, OUTPUT);
  digitalWrite(CAMERA_PIN, HIGH);

  setupSerial();
  setupSD();
  setupCamera();
}

void loop()
{
  led.loop();
  captureImage();
  delay(10000);
}
