package com.ais;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;
import com.fazecast.jSerialComm.SerialPortEvent;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.json.JSONObject;

import java.io.*;
import java.nio.charset.StandardCharsets;

public class LoRaToFirebaseBridge {
    private static SerialPort serialPort;
    private static CloseableHttpClient httpClient = HttpClients.createDefault();
    
    // Firebase URL
    private static final String FIREBASE_URL = 
        "https://ais-gps-tracker-default-rtdb.firebaseio.com/trackers/boat-001/latest.json";
    
    private static int packetCount = 0;
    private static long lastPacketTime = 0;

    public static void main(String[] args) {
        System.out.println("╔═══════════════════════════════════════════════════════╗");
        System.out.println("║     LoRa to Firebase Bridge - AIS GPS Tracker        ║");
        System.out.println("╚═══════════════════════════════════════════════════════╝");
        System.out.println();
        
        // List available ports
        SerialPort[] ports = SerialPort.getCommPorts();
        System.out.println("📌 Available Serial Ports:");
        for (int i = 0; i < ports.length; i++) {
            System.out.println("   " + i + ": " + ports[i].getSystemPortName() + 
                             " - " + ports[i].getDescriptivePortName());
        }
        System.out.println();
        
        // CHANGE THIS to match your Arduino's port
        String portName = "COM3"; // Windows
        // String portName = "/dev/ttyUSB0"; // Linux
        // String portName = "/dev/cu.usbmodem14101"; // Mac
        
        System.out.println("🔌 Attempting to connect to: " + portName);
        
        serialPort = SerialPort.getCommPort(portName);
        serialPort.setBaudRate(9600);
        serialPort.setNumDataBits(8);
        serialPort.setNumStopBits(1);
        serialPort.setParity(SerialPort.NO_PARITY);
        serialPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 100, 0);
        
        // Add data listener
        serialPort.addDataListener(new SerialPortDataListener() {
            @Override
            public int getListeningEvents() {
                return SerialPort.LISTENING_EVENT_DATA_AVAILABLE;
            }
            
            @Override
            public void serialEvent(SerialPortEvent event) {
                if (event.getEventType() != SerialPort.LISTENING_EVENT_DATA_AVAILABLE) {
                    return;
                }
                
                try {
                    byte[] buffer = new byte[1024];
                    int numRead = serialPort.readBytes(buffer, buffer.length);
                    
                    if (numRead > 0) {
                        String data = new String(buffer, 0, numRead, StandardCharsets.UTF_8);
                        processData(data);
                    }
                } catch (Exception e) {
                    System.err.println("❌ Error reading serial: " + e.getMessage());
                }
            }
        });
        
        // Open the port
        if (serialPort.openPort()) {
            System.out.println("✅ Serial port opened successfully!");
            System.out.println("📡 Listening for LoRa data...");
            System.out.println("   Press Ctrl+C to exit");
            System.out.println();
        } else {
            System.err.println("❌ Failed to open serial port!");
            System.err.println("   Make sure:");
            System.err.println("   1. Arduino is connected via USB");
            System.err.println("   2. No other program (like Arduino IDE) is using the port");
            System.err.println("   3. The port name is correct");
            return;
        }
        
        // Keep program running
        try {
            Thread.sleep(Long.MAX_VALUE);
        } catch (InterruptedException e) {
            System.out.println("\n🛑 Shutting down...");
        }
        
        // Cleanup
        serialPort.closePort();
        try {
            httpClient.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println("✅ Bridge stopped");
    }
    
    private static void processData(String rawData) {
        try {
            String[] lines = rawData.split("\n");
            for (String line : lines) {
                line = line.trim();
                if (line.isEmpty()) continue;
                
                if (line.startsWith("DATA:")) {
                    String dataLine = line.substring(5);
                    parseAndSendToFirebase(dataLine);
                }
            }
        } catch (Exception e) {
            System.err.println("❌ Error processing data: " + e.getMessage());
        }
    }
    
    private static void parseAndSendToFirebase(String dataLine) {
        try {
            String[] parts = dataLine.split(",");
            
            if (parts.length >= 4) {
                String name = parts[0].trim();
                double latitude = Double.parseDouble(parts[1].trim());
                double longitude = Double.parseDouble(parts[2].trim());
                int battery = Integer.parseInt(parts[3].trim());
                
                // Check for RSSI
                int rssi = -78; // Default
                if (parts.length >= 5 && parts[4].contains("RSSI:")) {
                    String rssiStr = parts[4].substring(parts[4].indexOf(":") + 1).trim();
                    rssi = Integer.parseInt(rssiStr);
                }
                
                packetCount++;
                lastPacketTime = System.currentTimeMillis();
                System.out.println("📦 Packet #" + packetCount + ": " + name + 
                                 " @ " + latitude + ", " + longitude + 
                                 " | Battery: " + battery + "% | RSSI: " + rssi + " dBm");
                
                sendToFirebase(name, latitude, longitude, rssi, battery);
                
            } else {
                System.err.println("⚠️ Invalid data format: " + dataLine);
            }
            
        } catch (NumberFormatException e) {
            System.err.println("⚠️ Number parsing error in: " + dataLine);
        } catch (Exception e) {
            System.err.println("❌ Error parsing data: " + e.getMessage());
        }
    }
    
    private static void sendToFirebase(String name, double lat, double lng, int rssi, int battery) {
        try {
            JSONObject json = new JSONObject();
            json.put("name", name);
            json.put("deviceId", "boat-001");
            json.put("latitude", lat);
            json.put("longitude", lng);
            json.put("accuracy", 5);
            json.put("battery", battery);
            json.put("rssi", rssi);
            json.put("timestamp", System.currentTimeMillis());
            
            String jsonString = json.toString();
            
            HttpPut putRequest = new HttpPut(FIREBASE_URL);
            putRequest.setHeader("Content-Type", "application/json");
            putRequest.setEntity(new StringEntity(jsonString, StandardCharsets.UTF_8));
            
            long startTime = System.currentTimeMillis();
            var response = httpClient.execute(putRequest);
            long endTime = System.currentTimeMillis();
            
            int statusCode = response.getStatusLine().getStatusCode();
            
            if (statusCode == 200) {
                System.out.println("   ✅ Firebase updated! (took " + (endTime - startTime) + "ms)");
            } else {
                System.err.println("   ❌ Firebase error: " + statusCode + " - " + 
                                 response.getStatusLine().getReasonPhrase());
            }
            
            response.close();
            
        } catch (Exception e) {
            System.err.println("❌ Error sending to Firebase: " + e.getMessage());
        }
    }
}