/* LoRa base receiver. Connect this board by USB to the computer running lora-bridge.
   Install: LoRa by Sandeep Mistry. */
#include <SPI.h>
#include <LoRa.h>
constexpr long LORA_FREQUENCY = 433E6; // Must exactly match the transmitter.
constexpr byte LORA_SS = 10, LORA_RST = 9, LORA_DIO0 = 2;
void setup() {
  Serial.begin(9600); LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQUENCY)) { Serial.println("LoRa startup failed"); while (true); }
  Serial.println("LoRa receiver ready");
}
void loop() {
  int size = LoRa.parsePacket(); if (!size) return;
  String packet; while (LoRa.available()) packet += (char)LoRa.read();
  // Java bridge expects: device,latitude,longitude,accuracy,battery,rssi
  Serial.println(packet + "," + String(LoRa.packetRssi()));
}
