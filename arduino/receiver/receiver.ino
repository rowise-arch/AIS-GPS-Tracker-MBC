#include <SPI.h>
#include <LoRa.h>

#define SS_PIN 10
#define RST_PIN 9
#define DIO0_PIN 2

int packetCount = 0;
unsigned long lastPacketTime = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("=================================");
  Serial.println("LoRa Receiver - Base Station");
  Serial.println("=================================");
  Serial.println();
  
  Serial.print("Initializing LoRa... ");
  if (!LoRa.begin(915E6)) {
    Serial.println("FAILED!");
    Serial.println("Check wiring:");
    Serial.println("  VCC → 3.3V, GND → GND");
    Serial.println("  NSS → D10, SCK → D13");
    Serial.println("  MOSI → D11, MISO → D12");
    Serial.println("  RST → D9, DIO0 → D2");
    while (1);
  }
  Serial.println("SUCCESS!");
  
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  
  Serial.println("LoRa configured:");
  Serial.println("  Frequency: 915 MHz");
  Serial.println("  Spreading Factor: 12");
  Serial.println("  Bandwidth: 125 kHz");
  Serial.println("  Coding Rate: 4/5");
  Serial.println();
  Serial.println("📡 Listening for packets...");
  Serial.println("=================================");
  Serial.println();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    packetCount++;
    String received = "";
    
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    int rssi = LoRa.packetRssi();
    lastPacketTime = millis();
    
    Serial.println("=================================");
    Serial.print("📦 Packet #");
    Serial.println(packetCount);
    Serial.print("   Data: ");
    Serial.println(received);
    Serial.print("   RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.println("=================================");
    Serial.println();
    
    // Forward to Serial for Python bridge
    Serial.print("DATA:");
    Serial.print(received);
    Serial.print(",RSSI:");
    Serial.println(rssi);
  }
  
  delay(100);
}