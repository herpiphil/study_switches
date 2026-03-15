# Study Switches — Project Context

## Last updated: 2026-03-15

## Hardware
- ESP32-H2 Mini (board: esp32-h2-devkitm-1), 4MB flash, no WiFi
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
- Framework: ESP-IDF (Arduino not supported for H2 Zigbee)
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
- v0.3.1: Compiled and flashed successfully via direct esptool call.
  Note: esphome run has a write-flash overlap bug with this platform — use
  esphome compile then flash manually (see How to Flash below).
- Not yet paired to Zigbee network

## How to Flash
`esphome run` has a write-flash overlap bug with this platform. Use:
```bash
esphome compile study_switches.yaml
/home/casg/.local/share/pipx/venvs/esphome/bin/python -m esptool \
  --port /dev/ttyACM0 --chip esp32h2 --baud 460800 write-flash 0x0 \
  .esphome/build/study-switches/.pioenvs/study-switches/firmware.factory.bin
```
Connect ESP32-H2 via USB before running.

## Re-pairing
Hold the All Off button (GPIO14) for 5 seconds to trigger factory reset and re-pair.

## Planned Future Work
- Rotary encoder for light intensity control
