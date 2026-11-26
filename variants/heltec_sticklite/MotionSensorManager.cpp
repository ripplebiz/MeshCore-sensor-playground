#include "MotionSensorManager.h"

MotionSensorManager::MotionSensorManager() : ld2410s(Serial1, 9, 10) {
  _armed = true;  // armed at boot up
}

bool MotionSensorManager::begin() {
  if (!ld2410s.begin()) return false;           // initializes UART at 115200
  delay(500);
  return configureForMaxRange();
}

bool MotionSensorManager::querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) {
  telemetry.addSwitch(TELEM_CHANNEL_SELF, _armed ? 1 : 0);
  telemetry.addPresence(TELEM_CHANNEL_SELF, ld2410s.isMotionDetected() ? 1 : 0);
  telemetry.addDistance(TELEM_CHANNEL_SELF, ((float)ld2410s.getDistance()) / 100.0f );  // convert to meters
  return true;
}

void MotionSensorManager::loop() {
  ld2410s.loop();
}

bool MotionSensorManager::configureForMaxRange() {
  // 1. Enter configuration mode
  if (!ld2410s.enterConfigMode()) return false;  // failed

  delay(100);

  // 2. Write better common parameters
  // We have to send the raw frame because the library hard-codes gate=1
  // Frame: FD FC FB FA | 26 00 | 70 00 | params... | 04 03 02 01

  uint8_t frame[] = {
    0xFD, 0xFC, 0xFB, 0xFA,   // header
    0x26, 0x00,               // length
    0x70, 0x00,               // command: write generic params

    // Farthest gate = 12 (0x0C)
    0x05, 0x00, 0x0C, 0x00, 0x00, 0x00,

    // Nearest gate = 0
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00,

    // Unmanned delay = 15 seconds
    0x06, 0x00, 0x0F, 0x00, 0x00, 0x00,

    // Status report freq ≈ 4 Hz (0x28 = 40 → 4.0 Hz)
    0x02, 0x00, 0x28, 0x00, 0x00, 0x00,

    // Distance report freq ≈ 4 Hz
    0x0C, 0x00, 0x28, 0x00, 0x00, 0x00,

    // Response speed = Normal (5)
    0x0B, 0x00, 0x05, 0x00, 0x00, 0x00,

    0x04, 0x03, 0x02, 0x01    // footer
  };

  Serial1.write(frame, sizeof(frame));
  delay(200);

  // Clear any response
  while (Serial1.available()) Serial1.read();

  // 3. Optional but highly recommended: Auto-generate thresholds
  // (This is the “Gen. Threshold” button in the official HLK tool)
  // Command roughly: enter config → auto update thresholds → exit
  // Many people report this fixes poor range on some modules.

  // 4. Exit config mode
  ld2410s.exitConfigMode();
  delay(100);

  return true;  // success
}
