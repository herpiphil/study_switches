# VolumeBox Keyboard Controller - Project Context

## Last updated: 2026-03-27

## Project Goal
Build a physical controller (buttons, rotary encoder, relay, buzzer, LED) that:
- Connects to Home Assistant via Zigbee (ESP32-C6)
- Sends USB HID keyboard/volume commands to a Windows PC (Raspberry Pi Pico)

## Hardware Architecture
- **ESP32-C6**: All physical I/O (buttons, rotary encoder, relay, buzzer, LED) + Zigbee to HA
- **Raspberry Pi Pico**: USB HID only - receives UART commands from C6, sends keystrokes to PC
- **C6 ↔ Pico**: UART serial connection
- **C6 ↔ HA**: Zigbee via Zigbee2MQTT (Z2MQTT)

## Current State (2026-03-27)
Phase 1 complete: Zigbee connectivity and button test working.

### What works
- C6 connects to Z2MQTT as a Zigbee end device
- Button on GPIO4 (active LOW, pull-up) reports occupancy true/false to Z2MQTT
- RGB LED on GPIO8 (WS2812B, neopixelWrite) gives visual feedback
- Factory reset via BOOT button (GPIO9) long press (3 seconds)

### Key lesson learned
`esp_zb_zcl_report_attr_cmd_req()` must use `ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT`
with dst address 0x0000 (coordinator) to send reports to Z2MQTT. Using the default
`ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT` (binding table) silently fails
because no binding exists until Z2MQTT configures one.

### Working file
`volbox_c6/volbox_c6.ino` - ESP32-C6 Arduino sketch

### Arduino CLI settings
- Board: ESP32C6 Dev Module
- Zigbee Mode: Zigbee ED (End Device)
- Partition Scheme: Zigbee 4MB with spiffs
- Arduino ESP32 core 3.x required
- Compile/flash:
  `arduino-cli compile/upload --fqbn "esp32:esp32:esp32c6:ZigbeeMode=ed,PartitionScheme=zigbee"`

## Next Steps
- Add remaining GPIO hardware (rotary encoder, relay, buzzer)
- Add UART output to Pico for HID commands
- Write Pico CircuitPython HID receiver code

## Original Source
- `volbox.py`: Original CircuitPython code for Raspberry Pi Pico (reference only)
