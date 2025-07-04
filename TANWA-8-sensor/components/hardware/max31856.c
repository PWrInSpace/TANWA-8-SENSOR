#include "max31856.h"

const char *TAG = "MAX31856";

void max31856_write_register(spi_device_handle_t spi_handle, uint8_t cs_pin, uint8_t address, uint8_t data) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address | 0x80};

    gpio_set_level(cs_pin, 0);
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.length = 8;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);   
    gpio_set_level(cs_pin, 1);
}

uint8_t max31856_read_register(spi_device_handle_t spi_handle, uint8_t cs_pin, uint8_t address) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address & 0x7F};

    gpio_set_level(cs_pin, 0);
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.length = 8;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = 0xFF;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    gpio_set_level(cs_pin, 1);
    uint8_t reg_value = spi_transaction.rx_data[0];
    return reg_value;
}

uint8_t max31856_read_fast_register(spi_device_handle_t spi_handle, uint8_t cs_pin, uint8_t address) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[2] = {address & 0x7F, 0xFF};

    gpio_set_level(cs_pin, 0);
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.length = 16;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    gpio_set_level(cs_pin, 1);
    uint8_t reg_value = spi_transaction.rx_data[0];
    return reg_value;
}

uint16_t max31856_read_register16(spi_device_handle_t spi_handle, uint8_t cs_pin, uint8_t address) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address & 0x7F};

    gpio_set_level(cs_pin, 0);
    spi_transaction.length = 8;
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b1 = spi_transaction.rx_data[0];

    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b2 = spi_transaction.rx_data[0];
    gpio_set_level(cs_pin, 1);

    uint16_t reg_value = ((b1 << 8) | b2);
    return reg_value;
}

uint32_t max31856_read_register24(spi_device_handle_t spi_handle, uint8_t cs_pin, uint8_t address) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address & 0x7F};

    gpio_set_level(cs_pin, 0);
    spi_transaction.length = 8;
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b1 = spi_transaction.rx_data[0];

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b2 = spi_transaction.rx_data[0];

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b3 = spi_transaction.rx_data[0];
    gpio_set_level(cs_pin, 1);

    uint32_t reg_value = ((b1 << 16) | (b2 << 8) | b3);
    return reg_value;
}

void max31856_oneshot_temperature(spi_device_handle_t spi_handle, uint8_t cs_pin) {
    max31856_write_register(spi_handle, cs_pin, MAX31856_CJTO_REG, 0x00);
    uint8_t val = max31856_read_register(spi_handle, cs_pin, MAX31856_CR0_REG);
    val &= ~MAX31856_CR0_AUTOCONVERT;
    val |= MAX31856_CR0_1SHOT;
    max31856_write_register(spi_handle, cs_pin, MAX31856_CR0_REG, val);
    vTaskDelay(5 / portTICK_PERIOD_MS);
}

void thermocouple_set_type(max31856_cfg *max31856, max31856_thermocoupletype_t tc_type) {
    uint8_t val = max31856_read_register(max31856->spi, max31856->cs_pin, MAX31856_CR1_REG);
    val &= 0xF0; // Mask off bottom 4 bits
    val |= (uint8_t)tc_type & 0x0F;
    max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_CR1_REG, val);
}

max31856_thermocoupletype_t thermocouple_get_type(max31856_cfg *max31856) {
    uint8_t val = max31856_read_register(max31856->spi, max31856->cs_pin, MAX31856_CR1_REG);
    val &= 0x0F;

    switch (val) {
        case MAX31856_TCTYPE_B: ESP_LOGI(TAG, "TC Type: B"); break;
        case MAX31856_TCTYPE_E: ESP_LOGI(TAG, "TC Type: E"); break;
        case MAX31856_TCTYPE_J: ESP_LOGI(TAG, "TC Type: J"); break;
        case MAX31856_TCTYPE_K: ESP_LOGI(TAG, "TC Type: K"); break;
        case MAX31856_TCTYPE_N: ESP_LOGI(TAG, "TC Type: N"); break;
        case MAX31856_TCTYPE_R: ESP_LOGI(TAG, "TC Type: R"); break;
        case MAX31856_TCTYPE_S: ESP_LOGI(TAG, "TC Type: S"); break;
        case MAX31856_TCTYPE_T: ESP_LOGI(TAG, "TC Type: T"); break;
        case MAX31856_VMODE_G8: ESP_LOGI(TAG, "Voltage x8 Gain mode"); break;
        case MAX31856_VMODE_G32: ESP_LOGI(TAG, "Voltage x8 Gain mode"); break;
        default: ESP_LOGI(TAG, "TC Type: Unknown"); break;
    }

    return (max31856_thermocoupletype_t)(val);
}

uint8_t thermocouple_read_fault(max31856_cfg *max31856, bool log_fault) {
    uint8_t fault_val = max31856_read_fast_register(max31856->spi, max31856->cs_pin, MAX31856_SR_REG);
    if (fault_val && log_fault) {
        if (fault_val & MAX31856_FAULT_CJRANGE) ESP_LOGI(TAG, "Fault: Cold Junction Range");
        if (fault_val & MAX31856_FAULT_TCRANGE) ESP_LOGI(TAG, "Fault: Thermocouple Range");
        if (fault_val & MAX31856_FAULT_CJHIGH) ESP_LOGI(TAG, "Fault: Cold Junction High");
        if (fault_val & MAX31856_FAULT_CJLOW) ESP_LOGI(TAG, "Fault: Cold Junction Low");
        if (fault_val & MAX31856_FAULT_TCHIGH) ESP_LOGI(TAG, "Fault: Thermocouple High");
        if (fault_val & MAX31856_FAULT_TCLOW) ESP_LOGI(TAG, "Fault: Thermocouple Low");
        if (fault_val & MAX31856_FAULT_OVUV) ESP_LOGI(TAG, "Fault: Over/Under Voltage");
        if (fault_val & MAX31856_FAULT_OPEN) ESP_LOGI(TAG, "Fault: Thermocouple Open");
    }
    max31856->fault = fault_val;
    return fault_val;
}

float thermocouple_read_coldjunction(max31856_cfg *max31856) {
    max31856_oneshot_temperature(max31856->spi, max31856->cs_pin);
    uint16_t cj_temp = max31856_read_register16(max31856->spi, max31856->cs_pin, MAX31856_CJTH_REG);
    // ESP_LOGI(TAG, "Thermo raw cold: %d", cj_temp);
    float cj_temp_float = cj_temp;
    cj_temp_float /= 256.0;
    max31856->coldjunction_c = cj_temp_float;
    max31856->coldjunction_f = (1.8 * cj_temp_float) + 32.0;
    return cj_temp_float;
}

float thermocouple_read_temperature(max31856_cfg *max31856) {
    max31856_oneshot_temperature(max31856->spi, max31856->cs_pin);
    uint32_t tc_temp = max31856_read_register24(max31856->spi, max31856->cs_pin, MAX31856_LTCBH_REG);
    // ESP_LOGI(TAG, "Thermo raw temp: %d", tc_temp);
    if (tc_temp & 0x800000) {
        tc_temp |= 0xFF000000; // fix sign bit
    }
    tc_temp >>= 5;  // bottom 5 bits are unused
    float tc_temp_float = tc_temp;
    tc_temp_float *= 0.0078125;
    max31856->thermocouple_c = tc_temp_float;
    max31856->thermocouple_f = (1.8 * tc_temp_float) + 32.0;
    //ESP_LOGI(TAG, "Thermo temp C: %f", tc_temp_float);
    return tc_temp_float;
}

void thermocouple_set_temperature_fault(max31856_cfg *max31856, float temp_low, float temp_high) {
    temp_low *= 16;
    temp_high *= 16;
    int16_t low = temp_low;
    int16_t high = temp_high;
    max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTHFTH_REG, high >> 8);
    max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTHFTL_REG, high);
    max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTLFTH_REG, low >> 8);
    max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTLFTL_REG, low);
}

bool max31856_init(max31856_cfg *max31856, uint8_t cs_pin) {
    gpio_config_t io_conf;
    io_conf.pull_down_en = 0;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL<<cs_pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(cs_pin, 1);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    esp_err_t ret;

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = (APB_CLK_FREQ/10), // 8 Mhz
        .dummy_bits = 0,
        .mode = 1,
        .flags = 0,
        .spics_io_num = -1, // Manually Control CS
        .queue_size = 1,
    };

    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &max31856->spi);
    ESP_ERROR_CHECK(ret);

    // Assert on All Faults
    max31856_write_register(max31856->spi, cs_pin, MAX31856_MASK_REG, 0x00);

    // Open Circuit Detection
    max31856_write_register(max31856->spi, cs_pin, MAX31856_CR0_REG, MAX31856_CR0_OCFAULT0);
    
    max31856->cs_pin = cs_pin;

    return true;
}
