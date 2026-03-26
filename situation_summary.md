# VolumeBox C6 — Situation Summary
# Date: 2026-03-27

## Project Intent

Build a physical controller box with buttons, rotary encoder, relay, buzzer and LED.
The device connects to Home Assistant (HA) via Zigbee using an ESP32-C6 microcontroller.
A Raspberry Pi Pico connected to the C6 via UART handles USB HID keyboard/volume
commands to a Windows PC.

The immediate goal is to get the ESP32-C6 working as a Zigbee device in HA via
Zigbee2MQTT (Z2MQTT), with a single button that can detect single press, double press
and long press, and trigger HA automations accordingly.

---

## Hardware

- **ESP32-C6 DevKitC** — all physical I/O + Zigbee end device
- **Raspberry Pi Pico** — USB HID only (not yet implemented)
- **Zigbee coordinator** — Z2MQTT running as a Home Assistant add-on
- **Button** — wired between GPIO4 and GND (internal pull-up)
- **RGB LED** — WS2812B on GPIO8 (requires neopixelWrite, not digitalWrite)
- **Factory reset** — BOOT button GPIO9 held 3 seconds

## Software / Toolchain

- Arduino ESP32 core 3.3.7
- arduino-cli installed at ~/bin/arduino-cli
- Compile/flash command:
  `arduino-cli compile/upload --fqbn "esp32:esp32:esp32c6:ZigbeeMode=ed,PartitionScheme=zigbee"`
- Port: /dev/ttyACM0

---

## What Worked

### 1. Zigbee connection to Z2MQTT
The C6 successfully joins the Z2MQTT network as a Zigbee end device.
Device MAC: 98:a3:16:ff:fe:9e:f9:20 (Zigbee address 0x98a316fffe9ef920)

### 2. RGB LED
neopixelWrite(RGB_PIN, r, g, b) works correctly on GPIO8.

### 3. Button detection
Single/double/long press state machine works correctly — confirmed by LED colour changes:
- Blue = single press detected
- Cyan = double press detected
- Purple = long press detected

### 4. Zigbee attribute reporting — ONE specific approach
Sending attribute reports ONLY works when using:

```cpp
cmd.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
cmd.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;  // coordinator
cmd.zcl_basic_cmd.dst_endpoint = 1;
```

The default `ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT` (binding table mode)
silently fails because no binding exists.

### 5. Single occupancy sensor reporting (one press type only)
Using ZigbeeOccupancySensor on endpoint 10 with the above direct-coordinator fix,
a single button press successfully updates the occupancy state in Z2MQTT.
This was demonstrated working — state changed from Clear to Occupied in Z2MQTT.

---

## What Did Not Work

### 1. Multiple sensor endpoints (3x ZigbeeOccupancySensor)
Attempting to use three ZigbeeOccupancySensor instances (endpoints 10, 11, 12)
for single/double/long press caused crashes (device rebooted) when reporting
from endpoints 11 or 12. Endpoint 10 alone was sometimes unreliable after
Z2MQTT re-interviewed the device.

### 2. ZigbeeMultistate
Crashed when used as end device — the official Espressif example for ZigbeeMultistate
explicitly requires ZIGBEE_ROUTER mode, not ZIGBEE_END_DEVICE.

### 3. ZigbeeContactSwitch (IAS Zone)
Joined Z2MQTT and appeared as "Alarm 1 / Alarm 2" entities, but state never changed
on button press. IAS Zone requires an enrollment handshake with the coordinator
before it can send zone status updates. This was not resolved.

### 4. ZigbeeTempSensor for numeric press type encoding
Attempted to report 1/2/3 as temperature values for single/double/long press.
Button detection and LED worked correctly but Z2MQTT showed "N/A C" —
reports did not reach Z2MQTT despite using the same direct-coordinator approach
that worked for the occupancy sensor.

### 5. Z2MQTT external converter
A converter JS file was written to map sensor values to action names (single/double/hold).
Installing it requires copying a file to the Z2MQTT config folder and editing
configuration.yaml — this was not completed due to complexity.

---

## Current State

The latest sketch uses ZigbeeTempSensor with direct coordinator reporting and
single/double/long press detection. Button LED feedback works. Z2MQTT shows
a temperature entity but reports are not received (N/A).

The last fully confirmed working state was: single ZigbeeOccupancySensor,
endpoint 10, direct coordinator report, hold button = occupied / release = clear.

---

## Key Technical Notes

- The Arduino ESP32 Zigbee library's `report()` methods all use binding table mode
  and silently fail without an established binding. Direct coordinator addressing
  must be used instead.
- `ZigbeeOccupancySensor.setReporting()` does not exist — it must be implemented
  manually using `esp_zb_zcl_update_reporting_info()` or via direct report.
- Z2MQTT re-interviewing a device can disrupt previously working report reception.
- After factory reset, Permit Join must be enabled in Z2MQTT for the device to rejoin.
- `neopixelWrite()` is built into the Arduino ESP32 core — no extra library needed.

---

## What Is Needed

A reliable method to send a value representing press type (single/double/long)
from an ESP32-C6 Zigbee end device to Z2MQTT, using the Arduino ESP32 core 3.x.

The value should trigger HA automations. It does not matter what Zigbee device
type is used (occupancy, temperature, contact, custom) as long as:
1. It works as a Zigbee END DEVICE (not router/coordinator)
2. Z2MQTT receives the value reliably
3. The value can differentiate between single, double and long press
4. It does not require complex Z2MQTT configuration or custom converters to function

A working Espressif example or known-good community project using ESP32-C6
as a Zigbee end device sensor reporting to Z2MQTT would be the ideal starting point.
