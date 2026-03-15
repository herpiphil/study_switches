# Study Switches — Project Context

## Last updated: 2026-03-15

## Hardware
- ESP32-C6 Mini (devkitc-1 compatible), 4MB flash
- 5 momentary push buttons wired GPIO → GND, internal pull-ups

## Pin Assignments
| Function | GPIO |
|----------|------|
| Light 1  | GPIO1  |
| Light 2  | GPIO10 |
| Light 3  | GPIO11 |
| Fan      | GPIO13 |
| All Off  | GPIO14 |

## Software
- ESPHome: 2026.2.2
- Framework: ESP-IDF (Arduino not supported for C6 Zigbee)
- External component: luar123/zigbee_esphome
- Zigbee role: End Device (not router)

## Files
| File | Purpose |
|------|---------|
| study_switches.yaml | Main ESPHome config |
| partitions_zb.csv   | Custom 4MB Zigbee partition table |
| secrets.yaml        | WiFi credentials (gitignored, not used by this device) |
| context.md          | This file |

## Current Status
- v0.1: YAML written, not yet compiled or flashed
- Not yet paired to Zigbee network

## How to Flash
```bash
esphome run study_switches.yaml
```
Connect ESP32-C6 via USB before running.

## Re-pairing
Hold the All Off button (GPIO14) for 5 seconds to trigger factory reset and re-pair.

## Planned Future Work
- Rotary encoder for light intensity control
