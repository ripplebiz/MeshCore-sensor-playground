#pragma once

#include <helpers/SensorManager.h>
#include "HLK-LD2410S.h"

class MotionSensorManager : public SensorManager {
  LD2410S ld2410s;
  bool _armed;
public:
  MotionSensorManager();
  bool begin() override;
  bool querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) override;
  void loop() override;
  bool configureForMaxRange();
  bool isMotionDetected() const { return ld2410s.isMotionDetected(); }
  bool isArmed() const { return _armed; }
  void setArmed(bool armed) { _armed = armed; }
};
