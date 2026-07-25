# Setup guide

## 1. Arduino

Install **LoRa** (Sandeep Mistry) and **TinyGPSPlus** libraries from Arduino Library Manager. Upload `arduino/transmitter/transmitter.ino` to the GPS device and `arduino/receiver/receiver.ino` to the base receiver. Set the legal frequency for your region in both sketches.

Open the base receiver Serial Monitor at 9600 baud. A valid received line looks like:

```text
boat-001,14.599512,120.984222,1.1,100,-78
```

Close Serial Monitor before running the bridge, because only one program can use the serial port.

## 2. Firebase

Create a Realtime Database. For a prototype, use these rules; lock them down with Firebase Authentication before deployment:

```json
{"rules":{"trackers":{"$trackerId":{"latest":{".read":true,".write":true}}}}}
```

## 3. Desktop bridge

Install Java 17+ and Maven. From `lora-bridge` run:

```text
mvn package
run.bat
```

Edit `run.bat` and change `COM3` to the COM port shown by Arduino IDE. The bridge writes each received packet to Firebase. A successful upload prints `Uploaded boat-001`.

## 4. Website

Register a Firebase web app and paste its complete config into `web-app/app.js`. Open `web-app` with a static server. The map shows the same `boat-001` path uploaded by the bridge.
