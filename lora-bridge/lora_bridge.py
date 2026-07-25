import serial
import requests
import json
import time
import sys
import os
import serial.tools.list_ports

# Firebase base URL
FIREBASE_BASE_URL = "https://ais-gps-tracker-default-rtdb.firebaseio.com/trackers"

# Serial port - CHANGE THIS TO YOUR ARDUINO PORT
PORT = "COM7"  # Change this if needed

# Store boat data for summary
boats = {}
packet_count = 0

def list_available_ports():
    """List all available serial ports"""
    print("📌 Available Serial Ports:")
    ports = serial.tools.list_ports.comports()
    for port in ports:
        print(f"   {port.device} - {port.description}")
    print()

def get_boat_name(boat_id):
    """Get display name for a boat"""
    boat_names = {
        "boat-001": "Boat 001",
        "boat-002": "Boat 002",
        "boat-003": "Boat 003",
        "boat-004": "Boat 004",
        "boat-005": "Boat 005",
        # Add more boats here as needed
    }
    return boat_names.get(boat_id, boat_id.upper())

def send_to_firebase(boat_id, name, lat, lng, rssi, battery):
    """Send GPS data to Firebase for a specific boat"""
    
    # Build the Firebase URL for this boat
    firebase_url = f"{FIREBASE_BASE_URL}/{boat_id}/latest.json"
    
    data = {
        "name": name,
        "deviceId": boat_id,
        "latitude": lat,
        "longitude": lng,
        "accuracy": 5,
        "battery": battery,
        "rssi": rssi,
        "timestamp": int(time.time() * 1000)
    }
    
    try:
        response = requests.put(firebase_url, json=data, timeout=5)
        if response.status_code == 200:
            print(f"   ✅ Firebase updated for {boat_id}!")
            return True
        else:
            print(f"   ❌ Firebase error for {boat_id}: {response.status_code}")
            return False
    except Exception as e:
        print(f"   ❌ Error: {e}")
        return False

def main():
    global packet_count
    
    print("╔═══════════════════════════════════════════════════════╗")
    print("║     LoRa to Firebase Bridge - AIS GPS Tracker        ║")
    print("║     Multi-Boat Support - Romblon, Philippines        ║")
    print("╚═══════════════════════════════════════════════════════╝")
    print()
    
    # List available ports
    list_available_ports()
    
    print(f"🔌 Attempting to connect to: {PORT}")
    print()
    
    try:
        # Open serial port
        ser = serial.Serial(PORT, 9600, timeout=1)
        time.sleep(2)
        print("✅ Serial port opened successfully!")
        print("📡 Listening for LoRa data from multiple boats...")
        print("   Press Ctrl+C to exit")
        print()
        print("=" * 60)
        print()
        
        firebase_success = 0
        firebase_fail = 0
        
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
                                # Parse: boat-xxx,lat,lng,battery,RSSI:value
                                boat_id = parts[0].strip()
                                lat = float(parts[1].strip())
                                lng = float(parts[2].strip())
                                battery = int(parts[3].strip())
                                
                                # Check for RSSI
                                rssi = -78
                                if len(parts) >= 5:
                                    if "RSSI:" in parts[4]:
                                        rssi = int(parts[4].split(":")[1].strip())
                                    else:
                                        try:
                                            rssi = int(parts[4].strip())
                                        except:
                                            pass
                                
                                packet_count += 1
                                
                                # Store boat data
                                boats[boat_id] = {
                                    'lat': lat,
                                    'lng': lng,
                                    'battery': battery,
                                    'rssi': rssi
                                }
                                
                                # Get boat name
                                name = get_boat_name(boat_id)
                                
                                # Display packet info
                                print(f"📦 Packet #{packet_count}")
                                print(f"   Boat: {name} ({boat_id})")
                                print(f"   Location: {lat:.6f}, {lng:.6f}")
                                print(f"   Battery: {battery}%")
                                print(f"   RSSI: {rssi} dBm")
                                
                                # Send to Firebase
                                if send_to_firebase(boat_id, name, lat, lng, rssi, battery):
                                    firebase_success += 1
                                else:
                                    firebase_fail += 1
                                
                                # Show active boats summary every 10 packets
                                if packet_count % 10 == 0:
                                    print()
                                    print(f"📊 Active Boats ({len(boats)}):")
                                    for bid in boats.keys():
                                        print(f"   • {get_boat_name(bid)} ({bid})")
                                    print()
                                
                                print("-" * 60)
                                print()
                            else:
                                print(f"⚠️ Invalid format: {data_line}")
                                print()
                        except ValueError as e:
                            print(f"⚠️ Parse error: {e}")
                            print(f"   Data: {data_line}")
                            print()
                    else:
                        # Print other serial output
                        if line and not line.startswith("LoRa"):
                            print(f"📝 {line}")
            
            time.sleep(0.05)
            
    except serial.SerialException as e:
        print(f"❌ Serial error: {e}")
        print()
        print("   Make sure:")
        print("   1. Arduino is connected via USB")
        print("   2. No other program is using the port")
        print("   3. The port name is correct")
    except KeyboardInterrupt:
        print("\n🛑 Shutting down...")
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("✅ Serial port closed")
        print("✅ Bridge stopped")
        print()
        print(f"📊 Final Statistics:")
        print(f"   Total packets: {packet_count}")
        print(f"   Firebase success: {firebase_success}")
        print(f"   Firebase failed: {firebase_fail}")
        print(f"   Active boats: {len(boats)}")
        if boats:
            print("   Boats tracked:")
            for bid in boats.keys():
                print(f"      - {get_boat_name(bid)} ({bid})")

if __name__ == "__main__":
    main()