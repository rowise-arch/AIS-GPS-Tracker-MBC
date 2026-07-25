# AIS GPS Tracker

Live GPS tracking with two LoRa-equipped Arduino boards, Firebase Realtime Database, and a web map.

```text
GPS + transmitter Arduino → LoRa → base receiver Arduino → USB serial → Java bridge → Firebase → web dashboard
```

| Folder | Purpose |
|---|---|
| `web-app/` | Live map dashboard |
| `lora-bridge/` | Java serial-to-Firebase uploader |
| `arduino/` | GPS transmitter and LoRa base receiver sketches |
| `docs/` | Wiring and setup instructions |

Start with [the setup guide](docs/setup-guide.md).
