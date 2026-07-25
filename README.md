# AIS GPS Tracker

A browser dashboard that shows the newest GPS location sent by your LoRa base station. It uses **Firebase Realtime Database**, so the map changes live without refreshing the page.

## Hardware data flow

```text
GPS Arduino + LoRa  --LoRa packet-->  Base Arduino + LoRa + Wi-Fi  --HTTPS-->  Firebase Realtime Database  --live listener-->  Website
```

> An Arduino Uno by itself cannot upload to Firebase because it has no Wi-Fi. Your base station needs an internet-capable board/module, such as an ESP32, ESP8266, Arduino Uno R4 WiFi, or Ethernet/Wi-Fi shield.

## Firebase setup

1. Create a project in the [Firebase Console](https://console.firebase.google.com/), then create a **Realtime Database**. Start in test mode while prototyping.
2. In **Project settings → Your apps**, add a web app and copy its Firebase configuration.
3. Replace the `firebaseConfig` values at the top of [app.js](app.js).
4. Keep `trackerPath` as `trackers/boat-001/latest`, or change it to the path used by your base station.
5. In the Firebase Realtime Database Rules tab, paste the contents of [database.rules.json](database.rules.json) for initial testing.
6. Serve this folder with any static web host (Firebase Hosting, GitHub Pages, Netlify, or VS Code Live Server). Do not open `index.html` directly from Windows Explorer because browser modules may be blocked on `file://` URLs.

## Data your base station must send

Write this JSON object to `/trackers/boat-001/latest.json` using the Firebase REST API. Update it after every valid LoRa/GPS packet.

```json
{
  "name": "Boat 001",
  "deviceId": "boat-001",
  "latitude": 14.599512,
  "longitude": 120.984222,
  "accuracy": 5,
  "battery": 86,
  "rssi": -78,
  "timestamp": 1784990000000
}
```

`timestamp` should be Unix time in milliseconds (for example, an ESP32 can get it from NTP). `rssi` is the LoRa received signal strength in dBm.

For a basic prototype, the base station can use an HTTPS `PUT` request to:

```text
https://YOUR_PROJECT_ID-default-rtdb.firebaseio.com/trackers/boat-001/latest.json
```

with the JSON above as the request body. For a real deployment, secure Firebase writes with Firebase Authentication or a server-side relay; the included test rules deliberately allow public writes and reads.

## Expected LoRa packet

The GPS Arduino can transmit a small comma-separated message, for example:

```text
boat-001,14.599512,120.984222,5,86
```

The base station parses it, adds `rssi` from the LoRa receiver and `timestamp`, then uploads the JSON to Firebase.
