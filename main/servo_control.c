#include "servo_control.h"
#include "driver/ledc.h"

#define SERVO_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_TIMER      LEDC_TIMER_0
#define SERVO_LEDC_CHANNEL    LEDC_CHANNEL_0
#define SERVO_LEDC_FREQ_HZ    50
#define SERVO_LEDC_RESOLUTION LEDC_TIMER_14_BIT

uint32_t servo_angle_to_duty(int angle) {
    // 1. 경계값 클램핑 (방어적 프로그래밍: 기구물 파손 방지)
    if (angle < SERVO_MIN_ANGLE) {
        angle = SERVO_MIN_ANGLE;
    } else if (angle > SERVO_MAX_ANGLE) {
        angle = SERVO_MAX_ANGLE;
    }

    // 2. 각도 -> 펄스 폭(us) 변환 (정수 오버플로우 방지를 위해 uint32_t 연산)
    uint32_t pulse_us = SERVO_MIN_PULSE_US +
        ((uint32_t)angle * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) / SERVO_MAX_ANGLE;

    // 3. 펄스 폭(us) -> 14-bit 듀티비 변환
    uint32_t duty = (pulse_us * SERVO_MAX_DUTY) / SERVO_PERIOD_US;

    return duty;
}

esp_err_t servo_init(int gpio_num) {
    // 1. LEDC 타이머 설정 (50Hz, 14-bit)
    ledc_timer_config_t timer_conf = {
        .speed_mode       = SERVO_LEDC_MODE,
        .duty_resolution  = SERVO_LEDC_RESOLUTION,
        .timer_num        = SERVO_LEDC_TIMER,
        .freq_hz          = SERVO_LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    esp_err_t err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK) {
        return err;
    }

    // 2. LEDC 채널 및 GPIO 핀 설정
    ledc_channel_config_t ch_conf = {
        .gpio_num       = gpio_num,
        .speed_mode     = SERVO_LEDC_MODE,
        .channel        = SERVO_LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = SERVO_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };
    return ledc_channel_config(&ch_conf);
}

esp_err_t servo_set_angle(int angle) {
    uint32_t duty = servo_angle_to_duty(angle);
    esp_err_t err = ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, duty);
    if (err != ESP_OK) {
        return err;
    }
    return ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);
}