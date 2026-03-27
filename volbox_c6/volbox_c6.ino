/*
 * Date: 2026-03-27
 * Project: VolumeBox C6 - Zigbee button, single press
 * Notes: ZigbeeOccupancySensor, endpoint 10.
 *        Direct coordinator report (0x0000) - required to bypass empty binding table.
 *        Button GPIO4 to GND: hold = occupied, release = clear.
 *        Factory reset: BOOT button (GPIO9) held 3 seconds.
 *
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

ZigbeeOccupancySensor zbButton = ZigbeeOccupancySensor(ENDPOINT);

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_PIN, r, g, b);
}

void reportOccupancy(bool occupied) {
  zbButton.setOccupancy(occupied);

  esp_zb_zcl_report_attr_cmd_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.address_mode                        = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  cmd.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;
  cmd.zcl_basic_cmd.dst_endpoint          = 1;
  cmd.zcl_basic_cmd.src_endpoint          = ENDPOINT;
  cmd.clusterID                           = ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING;
  cmd.attributeID                         = ESP_ZB_ZCL_ATTR_OCCUPANCY_SENSING_OCCUPANCY_ID;
  cmd.direction                           = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
  esp_zb_lock_acquire(portMAX_DELAY);
  esp_zb_zcl_report_attr_cmd_req(&cmd);
  esp_zb_lock_release();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BOOT_BTN, INPUT_PULLUP);
  setLED(0, 0, 0);

  zbButton.setManufacturerAndModel("VolumeBox", "C6-Switch-v1");
  Zigbee.addEndpoint(&zbButton);

  if (!Zigbee.begin()) {
    setLED(50, 0, 0); delay(1000); ESP.restart();
  }

  while (!Zigbee.connected()) {
    setLED(10, 10, 10); delay(300);
    setLED(0, 0, 0);    delay(700);
  }

  for (int i = 0; i < 3; i++) {
    setLED(0, 50, 0); delay(150);
    setLED(0, 0, 0);  delay(150);
  }
}

void loop() {
  static bool occupied = false;

  if (digitalRead(BUTTON_PIN) == LOW && !occupied) {
    setLED(0, 0, 50);
    reportOccupancy(true);
    occupied = true;
  }

  if (digitalRead(BUTTON_PIN) == HIGH && occupied) {
    setLED(0, 0, 0);
    reportOccupancy(false);
    occupied = false;
  }

  if (digitalRead(BOOT_BTN) == LOW) {
    delay(100);
    unsigned long t = millis();
    while (digitalRead(BOOT_BTN) == LOW) {
      delay(50);
      if ((millis() - t) > 3000) {
        setLED(50, 0, 0); delay(1000);
        Zigbee.factoryReset();
      }
    }
  }

  delay(10);
}
