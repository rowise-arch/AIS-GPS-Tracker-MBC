import serial
import requests
import json
import time
import sys
import os
import serial.tools.list_ports

# Firebase URL
FIREBASE_URL = "https://ais-gps-tracker-default-rtdb.firebaseio.com/trackers/boat-001/latest.json"

# Serial port - CHANGE THIS TO YOUR ARDUINO PORT
# Find your port by checking Device Manager or Arduino IDE
PORT = "COM7"  # Change this! Common: COM3, COM4, COM5

def list_available_ports():
    """List all available serial ports"""
    print("📌 Available Serial Ports:")
    ports = serial.tools.list_ports.comports()
    for port in ports:
        print(f"   {port.device} - {port.description}")
    print()

def send_to_firebase(name, lat, lng, rssi, battery):
    """Send GPS data to Firebase"""
    data = {
        "name": name,
        "deviceId": "boat-001",
        "latitude": lat,
        "longitude": lng,
        "accuracy": 5,
        "battery": battery,
        "rssi": rssi,
        "timestamp": int(time.time() * 1000)
    }
    
    try:
        response = requests.put(FIREBASE_URL, json=data, timeout=5)
        if response.status_code == 200:
            print("   ✅ Firebase updated!")
            return True
        else:
            print(f"   ❌ Firebase error: {response.status_code}")
            return False
    except requests.exceptions.Timeout:
        print("   ⏰ Timeout - check internet connection")
        return False
    except requests.exceptions.ConnectionError:
        print("   🔌 Cannot connect to Firebase - check internet")
        return False
    except Exception as e:
        print(f"   ❌ Error: {e}")
        return False

def main():
    print("╔═══════════════════════════════════════════════════════╗")
    print("║     LoRa to Firebase Bridge - Python Version         ║")
    print("║     AIS GPS Tracker                                  ║")
    print("╚═══════════════════════════════════════════════════════╝")
    print()
    
    # List available ports
    list_available_ports()
    
    # Check if port exists
    if not os.path.exists(PORT) and not PORT.startswith('COM'):
        print(f"⚠️ Port {PORT} not found!")
        print("   Please change the PORT variable to your Arduino port")
        print("   Common ports: COM3, COM4, COM5, /dev/ttyUSB0")
        print()
        print("   To find your port:")
        print("   1. Open Arduino IDE")
        print("   2. Go to Tools → Port")
        print("   3. See which COM port is selected")
        return
    
    try:
        # Open serial port
        print(f"🔌 Connecting to {PORT} at 9600 baud...")
        ser = serial.Serial(PORT, 9600, timeout=1)
        time.sleep(2)  # Wait for Arduino to reset
        print("✅ Serial port opened successfully!")
        print("📡 Listening for LoRa data...")
        print("   Press Ctrl+C to exit")
        print()
        print("=" * 60)
        
        packet_count = 0
        
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if line:
                    # Check if it's a DATA: line
                    if line.startswith("DATA:"):
                        data_line = line[5:]  # Remove "DATA:"
                        parts = data_line.split(",")
                        
                        try:
                            if len(parts) >= 4:
                                name = parts[0].strip()
                                lat = float(parts[1].strip())
                                lng = float(parts[2].strip())
                                battery = int(parts[3].strip())
                                
                                # Check for RSSI
                                rssi = -78  # Default
                                if len(parts) >= 5:
                                    if "RSSI:" in parts[4]:
                                        rssi = int(parts[4].split(":")[1].strip())
                                    else:
                                        try:
                                            rssi = int(parts[4].strip())
                                        except:
                                            pass
                                
                                packet_count += 1
                                print(f"📦 Packet #{packet_count}")
                                print(f"   Name: {name}")
                                print(f"   Location: {lat:.6f}, {lng:.6f}")
                                print(f"   Battery: {battery}%")
                                print(f"   RSSI: {rssi} dBm")
                                
                                # Send to Firebase
                                send_to_firebase(name, lat, lng, rssi, battery)
                                print("-" * 60)
                            else:
                                print(f"⚠️ Invalid format: {data_line}")
                                print(f"   Expected: name,lat,lng,battery")
                                print(f"   Received: {len(parts)} parts")
                        except ValueError as e:
                            print(f"⚠️ Parse error: {e}")
                            print(f"   Data: {data_line}")
                    else:
                        # Print other serial output for debugging
                        if line and not line.startswith("LoRa"):
                            print(f"📝 {line}")
            
            time.sleep(0.05)  # Small delay to prevent CPU overload
            
    except serial.SerialException as e:
        print(f"❌ Serial error: {e}")
        print()
        print("   Make sure:")
        print("   1. Arduino is connected via USB")
        print("   2. No other program (like Arduino IDE) is using the port")
        print("   3. The port name is correct")
        print()
        print("   To find your port:")
        print("   1. Open Arduino IDE")
        print("   2. Go to Tools → Port")
        print("   3. See which COM port is selected")
        print("   4. Update PORT = 'COMx' in the script")
    except KeyboardInterrupt:
        print("\n🛑 Shutting down...")
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("✅ Serial port closed")
        print("✅ Bridge stopped")

if __name__ == "__main__":
    main()