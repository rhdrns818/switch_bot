#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "mqtt_client.h"

#include "switch_parser.h"
#include "servo_control.h"

// 네트워크 및 하드웨어 설정
#define WIFI_SSID        "MY_WIFI_SSID"
#define WIFI_PASS        "MY_WIFI_PASSWORD"
#define MQTT_BROKER      "mqtt://192.168.0.46:1883"

#define TOPIC_SUB_SET    "robot/switch/set"
#define TOPIC_PUB_GET    "robot/switch/get"

#define SERVO_GPIO       4
#define SERVO_ANGLE_OFF  0
#define SERVO_ANGLE_ON   90

static const char *TAG = "SWITCH_BOT";
static esp_mqtt_client_handle_t mqtt_client = NULL;

// MQTT 이벤트 핸들러
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT 브로커 접속 성공! 토픽 구독: %s", TOPIC_SUB_SET);
            esp_mqtt_client_subscribe(mqtt_client, TOPIC_SUB_SET, 0);
            break;

        case MQTT_EVENT_DATA: {
            ESP_LOGI(TAG, "메시지 수신: 토픽=%.*s, 데이터=%.*s",
                     event->topic_len, event->topic, event->data_len, event->data);

            // 검증된 파서 모듈로 명령 분석
            switch_command_t cmd = parse_mqtt_command(
                event->topic, (size_t)event->topic_len,
                event->data, (size_t)event->data_len
            );

            if (cmd == CMD_ON) {
                ESP_LOGI(TAG, "[제어] 스위치 ON -> 서보 %d도 이동", SERVO_ANGLE_ON);
                if (servo_set_angle(SERVO_ANGLE_ON) == ESP_OK) {
                    esp_mqtt_client_publish(mqtt_client, TOPIC_PUB_GET, "true", 0, 0, 0);
                } else {
                    ESP_LOGE(TAG, "서보모터 제어 실패 (ON)");
                }
            } else if (cmd == CMD_OFF) {
                ESP_LOGI(TAG, "[제어] 스위치 OFF -> 서보 %d도 이동", SERVO_ANGLE_OFF);
                if (servo_set_angle(SERVO_ANGLE_OFF) == ESP_OK) {
                    esp_mqtt_client_publish(mqtt_client, TOPIC_PUB_GET, "false", 0, 0, 0);
                } else {
                    ESP_LOGE(TAG, "서보모터 제어 실패 (OFF)");
                }
            } else {
                ESP_LOGW(TAG, "유효하지 않은 명령 수신 (에러 코드: %d) -> 폐기(Drop)", cmd);
            }
            break;
        }

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT 브로커 연결 끊김");
            break;

        default:
            break;
    }
}

// Wi-Fi 이벤트 핸들러
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi 연결 끊김. 재연결 시도 중...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Wi-Fi 연결 성공! IP: " IPSTR, IP2STR(&event->ip_info.ip));

        if (mqtt_client != NULL) {
            esp_mqtt_client_start(mqtt_client);
        }
    }
}

void app_main(void) {
    // 1. NVS 초기화
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. 서보모터 초기화 및 초기 위치(OFF) 정렬
    ESP_ERROR_CHECK(servo_init(SERVO_GPIO));
    ESP_ERROR_CHECK(servo_set_angle(SERVO_ANGLE_OFF));
    ESP_LOGI(TAG, "서보모터 초기화 완료 (GPIO %d, 초기 각도 %d도)", SERVO_GPIO, SERVO_ANGLE_OFF);

    // 3. 네트워크 인터페이스 및 이벤트 루프 초기화
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 4. MQTT 클라이언트 사전 초기화 (Wi-Fi 연결 직후 안전하게 시작하기 위함)
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // 5. Wi-Fi 설정 및 시작
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}