#include "can_config.h"
#include "can_api.h"
#include "can_commands.h"
#include "BoardData.h"
#include "esp_log.h"
#include "esp_err.h"
#include "string.h"
#include "driver/twai.h"

#define TAG "CAN_CONFIG"

esp_err_t parse_float_to_int16_t(float * input, size_t size, int16_t *output)
{
    for(int i=0; i<size; i++)
    {
        output[i] =  input[i]*100.f; //MAX 327BAR due to int16_t range <-32767 to 32767>
    }
    return ESP_OK;
}
// Handler function definitions
esp_err_t send_board_status_handler(uint8_t *data, uint8_t length) 
{
    xSemaphoreTake(BoardDataSemaphore, pdMS_TO_TICKS(10));
    uint8_t data_send[3] = {0};
    data_send[0] = BoardData.status_temp;
    data_send[1] = BoardData.humidity; //TODO: ADD HUMIDITY DRIVER
    data_send[2] = 100; //TODO: ADD CURR SENSING ON ALL BOARDS
    xSemaphoreGive(BoardDataSemaphore);
    can_send_message(CAN_SEND_BOARD_STATUS, data_send, sizeof(data));
    return ESP_OK;
}

esp_err_t send_board_data_handler(uint8_t *data, uint8_t length) {

    return ESP_OK;
}

esp_err_t send_press_data_handler(uint8_t *data, uint8_t length) {

    int16_t pressure[8] = {0};
    uint8_t pressure_1_frame[8] = {0};
    uint8_t pressure_2_frame[8] = {0};
    xSemaphoreTake(BoardDataSemaphore, pdMS_TO_TICKS(10));
    parse_float_to_int16_t(BoardData.pressure, 8, pressure);

    memcpy(pressure_1_frame, &pressure[0], sizeof(pressure_1_frame));
    memcpy(pressure_2_frame, &pressure[4], sizeof(pressure_2_frame));


    can_send_message(CAN_SEND_PRESS_DATA_1, pressure_1_frame, sizeof(pressure_1_frame));
    can_send_message(CAN_SEND_PRESS_DATA_2, pressure_2_frame, sizeof(pressure_2_frame));
    xSemaphoreGive(BoardDataSemaphore);
    return ESP_OK;
}

esp_err_t send_temp_data_handler(uint8_t *data, uint8_t length) {

    int16_t temperature[3] = {0};
    uint8_t pt100_temp[2] = {35, 35};
    uint8_t frame[8] = {0};
    xSemaphoreTake(BoardDataSemaphore, pdMS_TO_TICKS(10));
    parse_float_to_int16_t(BoardData.temperature, 3, temperature);

    memcpy(frame, &temperature[0], sizeof(temperature));
    memcpy(frame+3, &pt100_temp, sizeof(pt100_temp));


    can_send_message(CAN_SEND_BOARD_STATUS, frame, sizeof(frame));
    xSemaphoreGive(BoardDataSemaphore);
    return ESP_OK;
}

esp_err_t send_press_info_handler(uint8_t *data, uint8_t length) {

    return ESP_OK;
}

esp_err_t send_press_data_rate_handler(uint8_t *data, uint8_t length) {

    return ESP_OK;
}




can_command_t can_commands[] = {
    {CAN_GET_STATUS, send_board_status_handler},
    {CAN_GET_BOARD_DATA, send_board_data_handler},
    {CAN_GET_PRESS_DATA, send_press_data_handler},
    {CAN_SET_PRESS_DATA_RATE, send_press_data_rate_handler},
    {CAN_GET_PRESS_INFO, send_press_info_handler}
};


esp_err_t can_config_init(void) {
    esp_err_t err;

    // Register CAN commands
    err = can_register_commands(can_commands, sizeof(can_commands) / sizeof(can_commands[0]));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN command registration failed");
        return err;
    }

    // Initialize CAN driver
    err = can_task_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN driver initialization failed");
        return err;
    }

    // Start CAN driver
    err = can_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN driver start failed");
        return err;
    }

    return ESP_OK;
}