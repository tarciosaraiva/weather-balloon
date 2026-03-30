#include <Arduino.h>
#include <LoRa.h>

constexpr int LORA_SS_PIN = 10;
constexpr int LORA_RESET_PIN = 9;
constexpr int LORA_DIO0_PIN = 2;
constexpr long LORA_FREQ = 915E6;
constexpr int SERIAL_PORT = 9700;

void setupSerial()
{
  Serial.begin(SERIAL_PORT);
  while (!Serial)
    ;
  Serial.println(F("Serial ready."));
}

void setupLoRa()
{
  LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
  while (!LoRa.begin(LORA_FREQ))
  {
    Serial.println(F("LoRa init failed, retrying..."));
    delay(1000);
  }
  LoRa.receive();
  Serial.println(F("LoRa ready."));
}

void setup()
{
  setupSerial();
  setupLoRa();
}

void loop()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize == 0)
    return;

  String packet = "";
  while (LoRa.available())
    packet += (char)LoRa.read();

  Serial.print(F("Received: "));
  Serial.println(packet);
  Serial.print(F("RSSI: "));
  Serial.println(LoRa.packetRssi());
}
