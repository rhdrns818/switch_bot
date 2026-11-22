#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <stdint.h>
#include "esp_err.h"

#define SERVO_MIN_ANGLE       0
#define SERVO_MAX_ANGLE       180
#define SERVO_MIN_PULSE_US    500     // 0도일 때 펄스 폭 (0.5ms)
#define SERVO_MAX_PULSE_US    2500    // 180도일 때 펄스 폭 (2.5ms)
#define SERVO_PERIOD_US       20000   // 50Hz 주기 (20ms)
#define SERVO_MAX_DUTY        (1U << 14) // LEDC 14-bit 해상도 (16384)

// 순수 연산 함수 (단위 테스트 대상)
uint32_t servo_angle_to_duty(int angle);

// 하드웨어 제어 함수 (LEDC 연동)
esp_err_t servo_init(int gpio_num);
esp_err_t servo_set_angle(int angle);

#endif // SERVO_CONTROL_H