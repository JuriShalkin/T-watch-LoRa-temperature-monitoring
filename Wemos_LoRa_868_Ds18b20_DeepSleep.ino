#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define BLYNK_PRINT Serial
#define ONE_WIRE_BUS D1
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors (&oneWire);

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");
  LoRa.setPins(15, 0, -1);

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.setSpreadingFactor(12);
  //LoRa.setSignalBandwidth(62.5E3);
  //LoRa.setCodingRate4(8);
  //LoRa.crc();
}

void loop() {
  
  Serial.print("Sending packet: ");
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.println(temp, 2);
  
  // send packet
  LoRa.beginPacket();
  LoRa.print(temp,2);
  LoRa.endPacket();
  LoRa.sleep();
  //delay(5000);
  ESP.deepSleep(60000000);
  
}
