#ifndef SWITCH_PARSER_H
#define SWITCH_PARSER_H

#include <stddef.h>

typedef enum {
    CMD_ERROR_NULL = -2,
    CMD_ERROR_INVALID = -1,
    CMD_OFF = 0,
    CMD_ON = 1
} switch_command_t;

// 수신된 MQTT 토픽과 페이로드를 분석하여 스위치 명령 반환
switch_command_t parse_mqtt_command(const char* topic, size_t topic_len, const char* payload, size_t payload_len);

#endif // SWITCH_PARSER_H