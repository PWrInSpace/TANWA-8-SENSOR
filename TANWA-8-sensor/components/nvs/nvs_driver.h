#include "nvs_flash.h"
#include "nvs.h"


typedef struct {
    struct {
        float p_max;
        float v_min;
        float v_max;
    } s[8]; // 8 sensorów
} nvs_cal_t;

esp_err_t save_nvs_cal();