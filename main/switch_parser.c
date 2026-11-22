#include "switch_parser.h"
#include <string.h>

switch_command_t parse_mqtt_command(const char* topic, size_t topic_len, const char* payload, size_t payload_len) {
    // 1. 널 포인터 검사 (방어적 프로그래밍)
    if (topic == NULL || payload == NULL) {
        return CMD_ERROR_NULL;
    }

    // 2. 토픽 검증 (지정된 제어 토픽인지 확인)
    const char* expected_topic = "robot/switch/set";
    size_t expected_topic_len = strlen(expected_topic);
    
    if (topic_len != expected_topic_len || strncmp(topic, expected_topic, topic_len) != 0) {
        return CMD_ERROR_INVALID;
    }

    // 3. 페이로드 분석 및 변환
    if (payload_len == 4 && strncmp(payload, "true", 4) == 0) {
        return CMD_ON;
    } else if (payload_len == 5 && strncmp(payload, "false", 5) == 0) {
        return CMD_OFF;
    }

    // 정의되지 않은 페이로드 수신 시 에러 처리
    return CMD_ERROR_INVALID;
}