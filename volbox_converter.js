// Date: 2026-03-27
// Project: VolumeBox C6 - Z2MQTT External Converter
// Notes: Maps temperature sensor value to action name.
//        1 = single, 2 = double, 3 = hold, 0 = idle (ignored).
//
// Install: copy to Z2MQTT config folder, add to configuration.yaml:
//   external_converters:
//     - volbox_converter.js

const ACTION_MAP = { 1: 'single', 2: 'double', 3: 'hold' };

const definition = {
  zigbeeModel: ['C6-Switch-v1'],
  model: 'C6-Switch-v1',
  vendor: 'VolumeBox',
  description: 'VolumeBox C6 button — single / double / hold',
  fromZigbee: [
    {
      cluster: 'msTemperatureMeasurement',
      type: ['attributeReport', 'readResponse'],
      convert: (model, msg, publish, options, meta) => {
        const value = Math.round(msg.data['measuredValue'] / 100);
        if (value === 0) return;
        const action = ACTION_MAP[value] || `unknown_${value}`;
        return { action };
      },
    },
  ],
  toZigbee: [],
  exposes: [
    require('zigbee-herdsman-converters/lib/exposes')
      .presets.action(['single', 'double', 'hold']),
  ],
};

module.exports = definition;
