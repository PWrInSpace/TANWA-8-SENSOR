#include "max31856.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t mutex_spi_tc;
const char *TAG = "MAX31856";

#define MAX31856_SPI_CLOCK_HZ 1000000
#define MAX31856_CONVERT_DELAY_MS 200

static esp_err_t max31856_transfer(spi_device_handle_t spi_handle,
                                   uint8_t cs_pin, const uint8_t *tx,
                                   uint8_t *rx, size_t len) {
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.length = len * 8;
  memcpy(t.tx_data, tx, len);

  gpio_set_level(cs_pin, 0);
  esp_err_t ret = spi_device_polling_transmit(spi_handle, &t);
  gpio_set_level(cs_pin, 1);

  if (ret == ESP_OK && rx != NULL) {
    memcpy(rx, t.rx_data, len);
  }
  return ret;
}

void max31856_write_register(spi_device_handle_t spi_handle, uint8_t cs_pin,
                             uint8_t address, uint8_t data) {
  uint8_t tx[2] = {(uint8_t)(address | 0x80), data};
  ESP_ERROR_CHECK(max31856_transfer(spi_handle, cs_pin, tx, NULL, sizeof(tx)));
}

uint8_t max31856_read_register(spi_device_handle_t spi_handle, uint8_t cs_pin,
                               uint8_t address) {
  uint8_t tx[2] = {(uint8_t)(address & 0x7F), 0x00};
  uint8_t rx[2] = {0};
  ESP_ERROR_CHECK(max31856_transfer(spi_handle, cs_pin, tx, rx, sizeof(tx)));
  return rx[1];
}

uint8_t max31856_read_fast_register(spi_device_handle_t spi_handle,
                                    uint8_t cs_pin, uint8_t address) {
  return max31856_read_register(spi_handle, cs_pin, address);
}

uint16_t max31856_read_register16(spi_device_handle_t spi_handle,
                                  uint8_t cs_pin, uint8_t address) {
  uint8_t tx[3] = {(uint8_t)(address & 0x7F), 0x00, 0x00};
  uint8_t rx[3] = {0};
  ESP_ERROR_CHECK(max31856_transfer(spi_handle, cs_pin, tx, rx, sizeof(tx)));
  return (uint16_t)((rx[1] << 8) | rx[2]);
}

uint32_t max31856_read_register24(spi_device_handle_t spi_handle,
                                  uint8_t cs_pin, uint8_t address) {
  uint8_t tx[4] = {(uint8_t)(address & 0x7F), 0x00, 0x00, 0x00};
  uint8_t rx[4] = {0};
  ESP_ERROR_CHECK(max31856_transfer(spi_handle, cs_pin, tx, rx, sizeof(tx)));
  return ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | rx[3];
}

void max31856_oneshot_temperature(spi_device_handle_t spi_handle,
                                  uint8_t cs_pin) {
  max31856_write_register(spi_handle, cs_pin, MAX31856_CJTO_REG, 0x00);
  uint8_t val = max31856_read_register(spi_handle, cs_pin, MAX31856_CR0_REG);
  val &= ~MAX31856_CR0_AUTOCONVERT;
  val |= MAX31856_CR0_1SHOT;
  max31856_write_register(spi_handle, cs_pin, MAX31856_CR0_REG, val);
  vTaskDelay(pdMS_TO_TICKS(MAX31856_CONVERT_DELAY_MS));
}

void thermocouple_set_type(max31856_cfg *max31856,
                           max31856_thermocoupletype_t tc_type) {
  uint8_t val =
      max31856_read_register(max31856->spi, max31856->cs_pin, MAX31856_CR1_REG);
  val &= 0xF0;
  val |= (uint8_t)tc_type & 0x0F;
  max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_CR1_REG,
                          val);
}

max31856_thermocoupletype_t thermocouple_get_type(max31856_cfg *max31856) {
  uint8_t val =
      max31856_read_register(max31856->spi, max31856->cs_pin, MAX31856_CR1_REG);
  val &= 0x0F;

  switch (val) {
  case MAX31856_TCTYPE_B:
    ESP_LOGI(TAG, "TC Type: B");
    break;
  case MAX31856_TCTYPE_E:
    ESP_LOGI(TAG, "TC Type: E");
    break;
  case MAX31856_TCTYPE_J:
    ESP_LOGI(TAG, "TC Type: J");
    break;
  case MAX31856_TCTYPE_K:
    ESP_LOGI(TAG, "TC Type: K");
    break;
  case MAX31856_TCTYPE_N:
    ESP_LOGI(TAG, "TC Type: N");
    break;
  case MAX31856_TCTYPE_R:
    ESP_LOGI(TAG, "TC Type: R");
    break;
  case MAX31856_TCTYPE_S:
    ESP_LOGI(TAG, "TC Type: S");
    break;
  case MAX31856_TCTYPE_T:
    ESP_LOGI(TAG, "TC Type: T");
    break;
  case MAX31856_VMODE_G8:
    ESP_LOGI(TAG, "Voltage x8 Gain mode");
    break;
  case MAX31856_VMODE_G32:
    ESP_LOGI(TAG, "Voltage x8 Gain mode");
    break;
  default:
    ESP_LOGI(TAG, "TC Type: Unknown");
    break;
  }

  return (max31856_thermocoupletype_t)(val);
}

uint8_t thermocouple_read_fault(max31856_cfg *max31856, bool log_fault) {
  uint8_t fault_val = max31856_read_fast_register(
      max31856->spi, max31856->cs_pin, MAX31856_SR_REG);
  if (fault_val && log_fault) {
    if (fault_val & MAX31856_FAULT_CJRANGE)
      ESP_LOGI(TAG, "Fault: Cold Junction Range");
    if (fault_val & MAX31856_FAULT_TCRANGE)
      ESP_LOGI(TAG, "Fault: Thermocouple Range");
    if (fault_val & MAX31856_FAULT_CJHIGH)
      ESP_LOGI(TAG, "Fault: Cold Junction High");
    if (fault_val & MAX31856_FAULT_CJLOW)
      ESP_LOGI(TAG, "Fault: Cold Junction Low");
    if (fault_val & MAX31856_FAULT_TCHIGH)
      ESP_LOGI(TAG, "Fault: Thermocouple High");
    if (fault_val & MAX31856_FAULT_TCLOW)
      ESP_LOGI(TAG, "Fault: Thermocouple Low");
    if (fault_val & MAX31856_FAULT_OVUV)
      ESP_LOGI(TAG, "Fault: Over/Under Voltage");
    if (fault_val & MAX31856_FAULT_OPEN)
      ESP_LOGI(TAG, "Fault: Thermocouple Open");
  }
  max31856->fault = fault_val;
  return fault_val;
}

float thermocouple_read_coldjunction(max31856_cfg *max31856) {
  uint16_t cj_temp = max31856_read_register16(max31856->spi, max31856->cs_pin,
                                              MAX31856_CJTH_REG);
  float cj_temp_float = cj_temp;
  cj_temp_float /= 256.0;
  max31856->coldjunction_c = cj_temp_float;
  max31856->coldjunction_f = (1.8 * cj_temp_float) + 32.0;
  return cj_temp_float;
}

float thermocouple_read_temperature(max31856_cfg *max31856) {
  uint32_t raw = max31856_read_register24(max31856->spi, max31856->cs_pin,
                                          MAX31856_LTCBH_REG);
  int32_t tc_temp = (int32_t)(raw << 8) >> 13;
  float tc_temp_float = (float)tc_temp * 0.0078125f;
  max31856->thermocouple_c = tc_temp_float;
  max31856->thermocouple_f = (1.8f * tc_temp_float) + 32.0f;
  return tc_temp_float;
}

void thermocouple_set_temperature_fault(max31856_cfg *max31856, float temp_low,
                                        float temp_high) {
  temp_low *= 16;
  temp_high *= 16;
  int16_t low = temp_low;
  int16_t high = temp_high;
  max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTHFTH_REG,
                          high >> 8);
  max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTHFTL_REG,
                          high);
  max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTLFTH_REG,
                          low >> 8);
  max31856_write_register(max31856->spi, max31856->cs_pin, MAX31856_LTLFTL_REG,
                          low);
}

bool max31856_init(max31856_cfg *max31856, uint8_t cs_pin,
                   spi_host_device_t host) {
  gpio_config_t io_conf;
  io_conf.pull_down_en = 0;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = (1ULL << cs_pin);
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);
  gpio_set_level(cs_pin, 1);

  vTaskDelay(pdMS_TO_TICKS(100));

  esp_err_t ret;

  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = MAX31856_SPI_CLOCK_HZ,
      .dummy_bits = 0,
      .mode = 1,
      .flags = SPI_DEVICE_NO_DUMMY,
      .spics_io_num = -1,
      .queue_size = 1,
  };

  xSemaphoreTake(mutex_spi_tc, portMAX_DELAY);
  ret = spi_bus_add_device(host, &devcfg, &max31856->spi);
  xSemaphoreGive(mutex_spi_tc);
  ESP_ERROR_CHECK(ret);

  max31856->cs_pin = cs_pin;

  xSemaphoreTake(mutex_spi_tc, portMAX_DELAY);
  max31856_write_register(max31856->spi, cs_pin, MAX31856_MASK_REG, 0x00);
  max31856_write_register(max31856->spi, cs_pin, MAX31856_CR0_REG,
                          MAX31856_CR0_AUTOCONVERT | MAX31856_CR0_OCFAULT0);
  xSemaphoreGive(mutex_spi_tc);

  return true;
}
