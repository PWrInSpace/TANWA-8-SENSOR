#include "mcp_driver.h"

#define V_BRIDGE 3.3f      
#define R_FIXED 100.0f // R65, R66, R53 = 100 Ohms
#define PT100_R0 100.0f // Resistance at 0°C

// Callendar-Van Dusen Constants
#define CVD_A 3.9083e-3f
#define CVD_B -5.775e-7f
#define CVD_C -4.183e-12f

static const char *TAG = "MCP_DRIVER";
static mcp342x_t mcp;

esp_err_t mcp_driver_init(void) {
    esp_err_t res = mcp342x_init_desc(&mcp, MCP_ADDR, MCP_I2C_PORT, MCP_I2C_SDA_PIN, MCP_I2C_SCL_PIN);
    if (res != ESP_OK) {
        return res;
    }

    mcp.mode = MCP342X_CONTINUOUS;
    mcp.resolution = MCP342X_RES_16;
    mcp.gain = MCP342X_GAIN4;

    return ESP_OK;
}

esp_err_t mcp_driver_read_voltage(mcp342x_channel_t channel, float *out_voltage) {
    mcp.channel = channel;

    return mcp342x_get_voltage(&mcp, out_voltage, NULL);
}

esp_err_t mcp_driver_read_PT100_temp(mcp342x_channel_t channel, float *out_temp) {
    float v_diff = 0;
    
    esp_err_t res = mcp_driver_read_voltage(channel, &v_diff);
    if (res != ESP_OK) return res;

    // WHEATSTONE BRIDGE CALCULATION
    float v_ratio = v_diff / V_BRIDGE;
    float r_measured = R_FIXED * (0.5f - v_ratio) / (0.5f + v_ratio);

    if (r_measured < 15.0f || r_measured > 450.0f) {
        ESP_LOGW(TAG, "PT100 sensor fault on channel %d (R = %.2f Ohm)", channel, r_measured);
        return ESP_ERR_INVALID_STATE;
    }

    // TEMPERATURE CONVERSION (Callendar-Van Dusen - IEC 60751):
    // For T >= 0°C: R(T) = R0[1 + AT + BT^2] for T.
    // For T < 0°C: R(T) = R0[1 + AT + BT^2 + C(T-100)T^3] for T.
    if (r_measured >= PT100_R0) { 
        // Positive temperatures
        *out_temp = (-CVD_A + sqrtf(CVD_A * CVD_A - 4 * CVD_B * (1 - r_measured / PT100_R0))) / (2 * CVD_B);
    } else { 
        // Negative temperatures
        // This polynomial is pre-calculated to match CVD_A, CVD_B and CVD_C constants.
        *out_temp = -242.02f + 2.2228f * r_measured + 2.5859e-3f * powf(r_measured, 2) - 4.8260e-6f * powf(r_measured, 3) - 2.8183e-8f * powf(r_measured, 4);
    }

    return ESP_OK;
}