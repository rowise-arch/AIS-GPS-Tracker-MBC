#include <SPI.h>
#include <LoRa.h>

// LoRa pins for Arduino Uno
#define SS_PIN 10
#define RST_PIN 9
#define DIO0_PIN 2

// ===== BOAT CONFIGURATION - Change this for each boat =====
const String BOAT_ID = "boat-001";      // ← Change for each boat
const String BOAT_NAME = "Boat 001";    // ← Change for each boat
// ==========================================================

// Romblon waypoints (simulated boat route)
struct Waypoint {
  float lat;
  float lng;
  const char* name;
};

// Define waypoints around Romblon
Waypoint waypoints[] = {
  {12.5742, 122.2709, "Romblon Town"},
  {12.5800, 122.2750, "North Romblon"},
  {12.5900, 122.2850, "Cresta de Gallo"},
  {12.5500, 122.2500, "Carabao Island"},
  {12.5200, 122.2400, "Logbon Island"},
  {12.5000, 122.2300, "Alad Island"},
  {12.4800, 122.2200, "Sibuyan Sea"},
  {12.4400, 122.1800, "Near Odiongan"},
  {12.4019, 122.0333, "Odiongan"},
  {12.4000, 122.0000, "Tablas Island"},
  {12.3444, 121.9778, "San Jose"},
  {12.3500, 122.7000, "Sibuyan Island"},
  {12.3667, 122.6833, "Cajidiocan"},
  {12.4833, 122.5167, "Magdiwang"}
};

int waypointCount = sizeof(waypoints) / sizeof(waypoints[0]);
int currentWaypoint = 0;
float currentLat = waypoints[0].lat;
float currentLng = waypoints[0].lng;
float step = 0;
int battery = 86;
int packetCount = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("=================================");
  Serial.println("LoRa Transmitter - " + BOAT_NAME);
  Serial.println("=================================");
  Serial.println();
  
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
  Serial.println("📍 Starting at: " + String(waypoints[0].name));
  Serial.println("   Lat: " + String(currentLat, 6) + ", Lng: " + String(currentLng, 6));
  Serial.println();
  Serial.println("📡 Sending packets every 2 seconds...");
  Serial.println("=================================");
  Serial.println();
}

void loop() {
  // Move along route
  step += 0.03;
  
  if (step >= 1.0) {
    step = 0;
    currentWaypoint = (currentWaypoint + 1) % waypointCount;
    currentLat = waypoints[currentWaypoint].lat;
    currentLng = waypoints[currentWaypoint].lng;
    
    Serial.println("⛵ Reached: " + String(waypoints[currentWaypoint].name));
    Serial.println("   Lat: " + String(currentLat, 6) + ", Lng: " + String(currentLng, 6));
  } else {
    int nextWaypoint = (currentWaypoint + 1) % waypointCount;
    float startLat = waypoints[currentWaypoint].lat;
    float startLng = waypoints[currentWaypoint].lng;
    float endLat = waypoints[nextWaypoint].lat;
    float endLng = waypoints[nextWaypoint].lng;
    
    float t = step * step * (3 - 2 * step);
    currentLat = startLat + (endLat - startLat) * t;
    currentLng = startLng + (endLng - startLng) * t;
  }
  
  // Simulate battery (85-100%)
  battery = 85 + random(0, 15);
  
  // Simulate RSSI (-90 to -60 dBm)
  int rssi = -90 + random(0, 30);
  
  packetCount++;
  
  // Format: BOAT_ID,latitude,longitude,battery,RSSI:value
  String data = BOAT_ID + ",";
  data += String(currentLat, 6) + ",";
  data += String(currentLng, 6) + ",";
  data += String(battery) + ",RSSI:";
  data += String(rssi);
  
  // Send via LoRa
  LoRa.beginPacket();
  LoRa.print(data);
  int result = LoRa.endPacket();
  
  if (result == 1) {
    Serial.print("📡 #");
    Serial.print(packetCount);
    Serial.print(": ");
    Serial.println(data);
  } else {
    Serial.println("❌ Send failed!");
  }
  
  delay(2000);
}