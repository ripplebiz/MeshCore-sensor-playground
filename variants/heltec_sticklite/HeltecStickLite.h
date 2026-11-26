#pragma once

#include <Arduino.h>
#include <helpers/ESP32Board.h>

// built-ins
#define  PIN_VBAT_READ   37
#define  PIN_LED_BUILTIN 25

class HeltecStickLiteBoard : public ESP32Board {
public:
  void begin() {
    ESP32Board::begin();

    pinMode(PIN_VBAT_READ, INPUT);
    adcAttachPin(PIN_VBAT_READ);

    pinMode(21, OUTPUT);

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_DEEPSLEEP) {
      long wakeup_source = esp_sleep_get_ext1_wakeup_status();
      if (wakeup_source & (1 << P_LORA_DIO_0)) {  // received a LoRa packet (while in deep sleep)
        startup_reason = BD_STARTUP_RX_PACKET;
      }

      rtc_gpio_hold_dis((gpio_num_t)P_LORA_NSS);
      rtc_gpio_deinit((gpio_num_t)P_LORA_DIO_0);
    }
  }

  uint16_t getBattMilliVolts() override {
    analogReadResolution(10);

    digitalWrite(21, LOW);
    delay(5);

    uint32_t raw = 0;
    for (int i = 0; i < 8; i++) {
      raw += analogRead(PIN_VBAT_READ);
    }
    raw = raw / 8;

    digitalWrite(21, HIGH);

    return (1.883 * (2 * 3.3 / 1024.0) * raw) * 1000;
  }

  const char* getManufacturerName() const override {
    return "Heltec V2";
  }

  uint32_t getIRQGpio() override {
    return P_LORA_DIO_0; // default for SX1276
  }
};
