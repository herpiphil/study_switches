/*
 * Date: 2026-03-27
 * Project: VolumeBox C6 - Zigbee Button Test
 * Notes: Replaced report() with direct send to coordinator (0x0000).
 *        Default report() uses binding table which is empty - reports go nowhere.
 *        Sending direct to coordinator short address bypasses this.
 *
 * Hardware: Button between GPIO4 and GND
 * Arduino IDE: Tools > Zigbee Mode > Zigbee ED (End Device)
 */

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"
#include "esp_zigbee_core.h"

#define SENSOR_PIN       4
#define BOOT_BTN         9
#define RGB_PIN          8
#define SENSOR_ENDPOINT  10

ZigbeeOccupancySensor zbOccupancySensor = ZigbeeOccupancySensor(SENSOR_ENDPOINT);

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_PIN, r, g, b);
}

// Send occupancy report directly to coordinator (short addr 0x0000)
// avoids the binding table which may be empty
void reportOccupancy() {
  esp_zb_zcl_report_attr_cmd_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.address_mode                    = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  cmd.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;  // coordinator
  cmd.zcl_basic_cmd.dst_endpoint      = 1;
  cmd.zcl_basic_cmd.src_endpoint      = SENSOR_ENDPOINT;
  cmd.clusterID                       = ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING;
  cmd.attributeID                     = ESP_ZB_ZCL_ATTR_OCCUPANCY_SENSING_OCCUPANCY_ID;
  cmd.direction                       = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
  cmd.manuf_specific                  = 0;
  cmd.dis_default_resp                = 0;
  esp_zb_lock_acquire(portMAX_DELAY);
  esp_zb_zcl_report_attr_cmd_req(&cmd);
  esp_zb_lock_release();
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(BOOT_BTN, INPUT_PULLUP);
  setLED(0, 0, 0);

  zbOccupancySensor.setManufacturerAndModel("VolumeBox", "C6-Switch-v1");
  Zigbee.addEndpoint(&zbOccupancySensor);

  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed - rebooting");
    setLED(50, 0, 0);
    delay(1000);
    ESP.restart();
  }

  Serial.println("Connecting...");
  while (!Zigbee.connected()) {
    Serial.print(".");
    setLED(10, 10, 10);
    delay(300);
    setLED(0, 0, 0);
    delay(700);
  }
  Serial.println("\nConnected");

  // 3 green blinks = ready
  for (int i = 0; i < 3; i++) {
    setLED(0, 50, 0);
    delay(150);
    setLED(0, 0, 0);
    delay(150);
  }
}

void loop() {
  static bool occupied = false;

  if (digitalRead(SENSOR_PIN) == LOW && !occupied) {
    Serial.println("Occupied - sending report to coordinator");
    setLED(0, 0, 50);
    zbOccupancySensor.setOccupancy(true);
    reportOccupancy();
    occupied = true;
  }

  if (digitalRead(SENSOR_PIN) == HIGH && occupied) {
    Serial.println("Not occupied - sending report to coordinator");
    setLED(0, 0, 0);
    zbOccupancySensor.setOccupancy(false);
    reportOccupancy();
    occupied = false;
  }

  // BOOT button long press = factory reset
  if (digitalRead(BOOT_BTN) == LOW) {
    delay(100);
    int startTime = millis();
    while (digitalRead(BOOT_BTN) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        Serial.println("Factory reset");
        setLED(50, 0, 0);
        delay(1000);
        Zigbee.factoryReset();
      }
    }
  }

  delay(100);
}
