# Wiring (Arduino Uno example)

## SX1276 / SX1278 LoRa module

| LoRa pin | Arduino Uno pin |
|---|---|
| VCC | **3.3V only** |
| GND | GND |
| SCK | D13 |
| MISO | D12 |
| MOSI | D11 |
| NSS / CS | D10 |
| RST | D9 |
| DIO0 | D2 |

Use a 3.3V logic-level converter when your LoRa board is not 5V tolerant. Do not power an SX127x module from 5V.

## NEO-6M GPS (transmitter only)

| GPS pin | Arduino Uno pin |
|---|---|
| VCC | 5V (or 3.3V if your module requires it) |
| GND | GND |
| TX | D4 |
| RX | D3 (optional) |

Both Arduino boards share the same LoRa wiring. Their LoRa frequency, spreading factor, and sync word must match.
