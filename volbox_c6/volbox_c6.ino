/*
 * Date: 2026-03-27
 * Project: VolumeBox C6 - Button single / double / long press
 * Notes: Single ZigbeeTempSensor endpoint - reports numeric value for press type.
 *        1 = single, 2 = double, 3 = long, 0 = idle.
 *        Direct coordinator report (0x0000) - proven working approach.
 *        Z2MQTT converter maps values to action names.
 *
 * Hardware: Button GPIO4 to GND. Factory reset: BOOT (GPIO9) held 3s.
 * Arduino IDE: Tools > Zigbee Mode > Zigbee ED (End Device)
 */

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"
#include "esp_zigbee_core.h"

#define BUTTON_PIN  4
#define BOOT_BTN    9
#define RGB_PIN     8
#define ENDPOINT    10

#define LONG_MS     800
#define DOUBLE_MS   400

ZigbeeTempSensor zbButton = ZigbeeTempSensor(ENDPOINT);

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_PIN, r, g, b);
}

void reportAction(float value) {
  zbButton.setTemperature(value);

  esp_zb_zcl_report_attr_cmd_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.address_mode                        = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  cmd.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;
  cmd.zcl_basic_cmd.dst_endpoint          = 1;
  cmd.zcl_basic_cmd.src_endpoint          = ENDPOINT;
  cmd.clusterID                           = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT;
  cmd.attributeID                         = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID;
  cmd.direction                           = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
  esp_zb_lock_acquire(portMAX_DELAY);
  esp_zb_zcl_report_attr_cmd_req(&cmd);
  esp_zb_lock_release();
}

void sendPress(float value, uint8_t r, uint8_t g, uint8_t b) {
  setLED(r, g, b);
  reportAction(value);
  delay(300);
  reportAction(0);
  setLED(0, 0, 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BOOT_BTN, INPUT_PULLUP);
  setLED(0, 0, 0);

  zbButton.setManufacturerAndModel("VolumeBox", "C6-Switch-v1");
  zbButton.setMinMaxValue(-10, 10);
  Zigbee.addEndpoint(&zbButton);

  if (!Zigbee.begin()) {
    setLED(50, 0, 0); delay(1000); ESP.restart();
  }

  while (!Zigbee.connected()) {
    setLED(10,10,10); delay(300); setLED(0,0,0); delay(700);
  }
  Serial.println("Connected");

  for (int i=0; i<3; i++) {
    setLED(0,50,0); delay(150); setLED(0,0,0); delay(150);
  }
}

void loop() {
  static enum { IDLE, PRESSED, WAIT_DOUBLE } state = IDLE;
  static unsigned long pressStart  = 0;
  static unsigned long releaseTime = 0;
  static bool longFired            = false;

  bool down = (digitalRead(BUTTON_PIN) == LOW);

  switch (state) {
    case IDLE:
      if (down) { pressStart = millis(); longFired = false; state = PRESSED; }
      break;

    case PRESSED:
      if (!down) {
        if (!longFired) { releaseTime = millis(); state = WAIT_DOUBLE; }
        else            { state = IDLE; }
      } else if (!longFired && (millis() - pressStart) >= LONG_MS) {
        Serial.println("Long");
        sendPress(3, 50, 0, 50);   // purple
        longFired = true;
      }
      break;

    case WAIT_DOUBLE:
      if (down) {
        Serial.println("Double");
        sendPress(2, 0, 50, 50);   // cyan
        while (digitalRead(BUTTON_PIN) == LOW) delay(10);
        state = IDLE;
      } else if ((millis() - releaseTime) >= DOUBLE_MS) {
        Serial.println("Single");
        sendPress(1, 0, 0, 50);    // blue
        state = IDLE;
      }
      break;
  }

  if (digitalRead(BOOT_BTN) == LOW) {
    delay(100); unsigned long t = millis();
    while (digitalRead(BOOT_BTN) == LOW) {
      delay(50);
      if ((millis()-t) > 3000) {
        setLED(50,0,0); delay(1000); Zigbee.factoryReset();
      }
    }
  }

  delay(10);
}
