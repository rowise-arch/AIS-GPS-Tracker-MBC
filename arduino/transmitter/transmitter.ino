/* GPS + LoRa transmitter. Assumes Arduino Uno, NEO-6M GPS, SX1276/SX1278 LoRa.
   Install: LoRa by Sandeep Mistry, TinyGPSPlus by Mikal Hart. */
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

constexpr long LORA_FREQUENCY = 433E6; // Change to 868E6 or 915E6 to match your legal LoRa band.
constexpr byte LORA_SS = 10, LORA_RST = 9, LORA_DIO0 = 2;
constexpr byte GPS_RX = 4, GPS_TX = 3; // Arduino RX, TX; GPS TX connects to GPS_RX.
const char DEVICE_ID[] = "boat-001";
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
unsigned long lastSent = 0;

void setup() {
  Serial.begin(9600); gpsSerial.begin(9600);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQUENCY)) { Serial.println("LoRa startup failed"); while (true); }
  LoRa.setTxPower(17); Serial.println("GPS LoRa transmitter ready");
}
void loop() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());
  if (millis() - lastSent < 5000 || !gps.location.isValid() || gps.location.age() > 3000) return;
  lastSent = millis();
  // Battery measurement is board-specific; replace 100 with a real percentage if required.
  String packet = String(DEVICE_ID) + "," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "," + String(gps.hdop.hdop() / 100.0, 1) + ",100";
  LoRa.beginPacket(); LoRa.print(packet); LoRa.endPacket();
  Serial.println("Sent: " + packet);
}
