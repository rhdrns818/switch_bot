#include "unity.h"
#include "switch_parser.h"
#include "servo_control.h"

void setUp(void) {}
void tearDown(void) {}

void test_parse_valid_commands(void) {
    const char* topic = "robot/switch/set";
    TEST_ASSERT_EQUAL(CMD_ON, parse_mqtt_command(topic, 16, "true", 4));
    TEST_ASSERT_EQUAL(CMD_OFF, parse_mqtt_command(topic, 16, "false", 5));
}

void test_parse_invalid_payload(void) {
    const char* topic = "robot/switch/set";
    TEST_ASSERT_EQUAL(CMD_ERROR_INVALID, parse_mqtt_command(topic, 16, "True", 4));
    TEST_ASSERT_EQUAL(CMD_ERROR_INVALID, parse_mqtt_command(topic, 16, "1", 1));
    TEST_ASSERT_EQUAL(CMD_ERROR_INVALID, parse_mqtt_command(topic, 16, "hello", 5));
}

void test_parse_boundary_and_null(void) {
    const char* topic = "robot/switch/set";
    TEST_ASSERT_EQUAL(CMD_ERROR_INVALID, parse_mqtt_command(topic, 16, "", 0));
    TEST_ASSERT_EQUAL(CMD_ERROR_INVALID, parse_mqtt_command(topic, 16, "true_but_too_long", 17));
    TEST_ASSERT_EQUAL(CMD_ERROR_NULL, parse_mqtt_command(NULL, 0, "true", 4));
    TEST_ASSERT_EQUAL(CMD_ERROR_NULL, parse_mqtt_command(topic, 16, NULL, 0));
}

void test_servo_valid_angles(void) {
    TEST_ASSERT_EQUAL_UINT32(409, servo_angle_to_duty(0));
    TEST_ASSERT_EQUAL_UINT32(1228, servo_angle_to_duty(90));
    TEST_ASSERT_EQUAL_UINT32(2048, servo_angle_to_duty(180));
}

void test_servo_out_of_bounds_clamping(void) {
    TEST_ASSERT_EQUAL_UINT32(409, servo_angle_to_duty(-1));
    TEST_ASSERT_EQUAL_UINT32(409, servo_angle_to_duty(-100));
    TEST_ASSERT_EQUAL_UINT32(2048, servo_angle_to_duty(181));
    TEST_ASSERT_EQUAL_UINT32(2048, servo_angle_to_duty(360));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_valid_commands);
    RUN_TEST(test_parse_invalid_payload);
    RUN_TEST(test_parse_boundary_and_null);
    RUN_TEST(test_servo_valid_angles);
    RUN_TEST(test_servo_out_of_bounds_clamping);
    return UNITY_END();
}