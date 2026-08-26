#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// LoRa pins for Arduino Uno
#define SS_PIN 10
#define RST_PIN 9
#define DIO0_PIN 2

// GPS pins
#define GPS_RX_PIN 4  // GPS TX → Arduino RX (pin 4)
#define GPS_TX_PIN 3  // GPS RX → Arduino TX (pin 3)

// ===== BOAT CONFIGURATION - Change this for each boat =====
const String BOAT_ID = "boat-001";      // ← Change for each boat
const String BOAT_NAME = "Boat 001";    // ← Change for each boat
// ==========================================================

// Create GPS and SoftwareSerial objects
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

// Variables
int battery = 86;
int packetCount = 0;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // Send every 5 seconds

// For GPS status tracking
bool gpsFixed = false;
unsigned long lastGpsUpdate = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("=================================");
  Serial.println("LoRa Transmitter - " + BOAT_NAME);
  Serial.println("=================================");
  Serial.println();
  
  // Initialize GPS
  Serial.print("Initializing GPS... ");
  gpsSerial.begin(9600);
  Serial.println("OK");
  
  // Initialize LoRa (915MHz for Philippines)
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
  
  // Configure LoRa for long range
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  
  Serial.println("LoRa configured:");
  Serial.println("  Frequency: 915 MHz");
  Serial.println("  Spreading Factor: 12");
  Serial.println("  Bandwidth: 125 kHz");
  Serial.println("  Coding Rate: 4/5");
  Serial.println();
  Serial.println("📍 Boat ID: " + BOAT_ID);
  Serial.println();
  Serial.println("📡 Waiting for GPS fix...");
  Serial.println("   (This may take 1-3 minutes on first startup)");
  Serial.println();
  Serial.println("📡 Sending packets every 5 seconds...");
  Serial.println("=================================");
  Serial.println();
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
    
    // Update GPS status
    if (gps.location.isValid()) {
      gpsFixed = true;
      lastGpsUpdate = millis();
    }
  }
  
  // Check if GPS has lost fix (no data for 10 seconds)
  if (gpsFixed && millis() - lastGpsUpdate > 10000) {
    gpsFixed = false;
    Serial.println("⚠️ GPS signal lost!");
  }
  
  // Send data at interval
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    sendData();
  }
  
  delay(100);
}

void sendData() {
  // Get battery reading (replace with actual analog reading)
  battery = readBattery();
  
  // Get RSSI from LoRa (will be updated when packet is sent)
  int loraRssi = LoRa.packetRssi();
  
  packetCount++;
  
  // Format data
  String data = "";
  
  if (gps.location.isValid()) {
    // GPS has valid data
    double lat = gps.location.lat();
    double lng = gps.location.lng();
    float speed = gps.speed.kmph(); // Speed in km/h
    float altitude = gps.altitude.meters();
    int satellites = gps.satellites.value();
    
    // Format: BOAT_ID,latitude,longitude,speed,altitude,satellites,battery,RSSI:value
    data = BOAT_ID + ",";
    data += String(lat, 6) + ",";
    data += String(lng, 6) + ",";
    data += String(speed, 1) + ",";        // Speed in km/h
    data += String(altitude, 1) + ",";     // Altitude in meters
    data += String(satellites) + ",";      // Number of satellites
    data += String(battery) + ",RSSI:";
    data += String(loraRssi);
    
    // Print to serial with GPS info
    Serial.print("📡 #");
    Serial.print(packetCount);
    Serial.print(": ");
    Serial.print(data);
    Serial.print(" | GPS: ");
    Serial.print(lat, 6);
    Serial.print(", ");
    Serial.print(lng, 6);
    Serial.print(" | Speed: ");
    Serial.print(speed, 1);
    Serial.print(" km/h | Sats: ");
    Serial.println(satellites);
    
  } else {
    // No GPS fix - send last known or default location
    data = BOAT_ID + ",";
    data += "0.000000,0.000000,0.0,0.0,0,";  // No GPS data
    data += String(battery) + ",RSSI:";
    data += String(loraRssi);
    
    Serial.print("📡 #");
    Serial.print(packetCount);
    Serial.print(": ");
    Serial.print(data);
    Serial.println(" | ❌ NO GPS FIX");
    
    // Print GPS status for debugging
    printGpsStatus();
  }
  
  // Send via LoRa
  LoRa.beginPacket();
  LoRa.print(data);
  int result = LoRa.endPacket();
  
  if (result != 1) {
    Serial.println("❌ LoRa send failed!");
  }
}

int readBattery() {
  // Read battery voltage from pin A2 (with voltage divider)
  // Replace with your actual battery reading circuit
  int sensorValue = analogRead(A2);
  
  // Convert to voltage (assuming 5V reference and voltage divider)
  // Example: 10k + 10k divider gives 2x multiplication
  float voltage = (sensorValue / 1023.0) * 5.0 * 2.0;
  
  // Convert to percentage (assuming 3.0V = 0%, 4.2V = 100%)
  int percentage = map(voltage * 100, 300, 420, 0, 100);
  percentage = constrain(percentage, 0, 100);
  
  return percentage;
}

void printGpsStatus() {
  Serial.print("   GPS Status - ");
  if (gps.charsProcessed() < 10) {
    Serial.println("No GPS data received yet");
    Serial.println("   Check GPS wiring (VCC, GND, TX→RX)");
    return;
  }
  
  Serial.print("Satellites: ");
  Serial.print(gps.satellites.value());
  Serial.print(" | HDOP: ");
  Serial.print(gps.hdop.value());
  Serial.print(" | Age: ");
  Serial.print(gps.location.age());
  Serial.println("ms");
}

// Function to force GPS update if needed
void forceGpsUpdate() {
  // If no GPS data for 30 seconds, restart GPS communication
  if (millis() - lastGpsUpdate > 30000 && !gps.location.isValid()) {
    Serial.println("🔄 Restarting GPS...");
    gpsSerial.begin(9600);
    lastGpsUpdate = millis();
  }
}