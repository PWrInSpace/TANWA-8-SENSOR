#include "nvs_driver.h"
#include "console.h"
#include "board_config.h"
esp_err_t save_nvs_cal()
{
nvs_cal_t cal;
    
    for (int i = 0; i < 4; i++) {
        cal.s[i].p_max = config.pressure_driver[0].sensors[i].pressure_max;
        cal.s[i].v_min = config.pressure_driver[0].sensors[i].voltage_min;
        cal.s[i].v_max = config.pressure_driver[0].sensors[i].voltage_max;
        
        cal.s[i+4].p_max = config.pressure_driver[1].sensors[i].pressure_max;
        cal.s[i+4].v_min = config.pressure_driver[1].sensors[i].voltage_min;
        cal.s[i+4].v_max = config.pressure_driver[1].sensors[i].voltage_max;
    }

    nvs_handle_t h;
    if (nvs_open("storage", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_blob(h, "cal_only", &cal, sizeof(nvs_cal_t));
        nvs_commit(h);
        nvs_close(h);
    }
    return ESP_OK;
}