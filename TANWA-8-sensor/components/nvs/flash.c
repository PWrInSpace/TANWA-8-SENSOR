#include "flash.h"

// create runtime_config
static data_config_t runtime_config = {0};

static const char *TAG = "FLASH";
static SemaphoreHandle_t runtime_mutex = NULL;

esp_err_t flash_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if (!runtime_mutex) {
        runtime_mutex = xSemaphoreCreateMutex();
        if (!runtime_mutex) {
            ESP_LOGE(TAG, "Failed to create runtime_config mutex!");
            return ESP_ERR_NO_MEM;
        }
    }

    ret = flash_read(&runtime_config);
    return ret;
}

esp_err_t flash_restore_defaults(void) {
    if (xSemaphoreTake(runtime_mutex, portMAX_DELAY) != pdTRUE) return ESP_ERR_TIMEOUT;

    runtime_config = (data_config_t){
        #define DATA(name, type, default_val) .name = default_val,
        #define DATA_ARRAY(name, type, size, default_val) .name = default_val,
        #define SECTION_BEGIN(name) .name = {
        #define SECTION_END(name) },

        CONFIG_FIELDS

        #undef DATA
        #undef DATA_ARRAY
        #undef SECTION_BEGIN
        #undef SECTION_END
    };

    xSemaphoreGive(runtime_mutex);
    return ESP_OK;
}

esp_err_t flash_erase_config(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_key(handle, BLOB_KEY);
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

esp_err_t flash_erase_all(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_all(handle);
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

esp_err_t flash_commit(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    if (xSemaphoreTake(runtime_mutex, portMAX_DELAY) != pdTRUE) return ESP_ERR_TIMEOUT;
    err = nvs_set_blob(handle, BLOB_KEY, &runtime_config, sizeof(data_config_t));
    xSemaphoreGive(runtime_mutex);

    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

esp_err_t flash_get_runtime_config(data_config_t *out_config) {
    if (xSemaphoreTake(runtime_mutex, portMAX_DELAY) != pdTRUE) return ESP_ERR_TIMEOUT;
    *out_config = runtime_config;
    xSemaphoreGive(runtime_mutex);

    return ESP_OK;
}

esp_err_t flash_read(data_config_t *out_config) {
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) return ret;

    size_t required_size = sizeof(data_config_t);
    ret = nvs_get_blob(handle, BLOB_KEY, out_config, &required_size);

    if (ret != ESP_OK) {
        memset(out_config, 0, sizeof(data_config_t));
    }

    nvs_close(handle);
    return ret;
}

esp_err_t flash_edit_config(data_config_t config) {
    if (xSemaphoreTake(runtime_mutex, portMAX_DELAY) != pdTRUE) return ESP_ERR_TIMEOUT;
    runtime_config = config;
    xSemaphoreGive(runtime_mutex);

    return ESP_OK;
}

//CRITICAL FIX NEEDED
const char **flash_get_field_names(size_t *count) {
    #define STR2(x) #x
    #define STR(x) STR2(x)
    #define JOIN(a,b) a "." b
    #define SECTION_PREFIX ""

    #define DATA(name, type, default_val) \
        (SECTION_PREFIX[0] ? JOIN(SECTION_PREFIX, STR(name)) : STR(name)),
    #define DATA_ARRAY(name, type, size, default_val) \
        (SECTION_PREFIX[0] ? JOIN(SECTION_PREFIX, STR(name)) : STR(name)),
    #define SECTION_BEGIN(name) \
        #undef SECTION_PREFIX \
        #define SECTION_PREFIX STR(name)
    #define SECTION_END(name) \
        #undef SECTION_PREFIX \
        #define SECTION_PREFIX ""

    static const char *names[] = {
        CONFIG_FIELDS
    };

    #undef STR2
    #undef STR
    #undef JOIN
    #undef DATA
    #undef DATA_ARRAY
    #undef SECTION_BEGIN
    #undef SECTION_END
    #undef CURRENT_SECTION

    if (count) *count = sizeof(names)/sizeof(names[0]);
    return names;
}

static esp_err_t parse_int32_t(const char *value, int32_t *out);
static esp_err_t parse_uint8_t(const char *value, uint8_t *out);
static esp_err_t parse_float(const char *value, float *out);
static esp_err_t parse_double(const char *value, double *out);
static esp_err_t parse_char(const char *value, char *out);
static esp_err_t parse_string(const char *value, char *out, size_t max_size);

typedef struct {
    const char *name;
    const char *type;
    size_t offset;
    size_t field_size;
} field_map_t;

esp_err_t update_field(data_config_t *config, field_map_t field, const char *value) {
    if (!config || !value) return ESP_ERR_INVALID_ARG;
    uint8_t *dest = (uint8_t*)config + field.offset;

    // Try parsing arrays first
    if (strstr(field.type, "char[") != NULL) { return parse_string(value, (char*)dest, field.field_size); }

    // Try parsing standard types
    if (strcmp(field.type, "int32_t") == 0) { return parse_int32_t(value, (int32_t*)dest); }
    if (strcmp(field.type, "uint8_t") == 0) { return parse_uint8_t(value, (uint8_t*)dest); }
    if (strcmp(field.type, "double") == 0) { return parse_double(value, (double*)dest); }
    if (strcmp(field.type, "float") == 0) { return parse_float(value, (float*)dest); }
    if (strcmp(field.type, "char") == 0) { return parse_char(value, (char*)dest); }

    return ESP_ERR_NOT_SUPPORTED;
}

//CRITICAL FIX NEEDED
esp_err_t flash_edit_field(const char *field_name, const char *value) {
    if (!value || !field_name) return ESP_ERR_INVALID_ARG;

    esp_err_t ret;
    data_config_t updated_config;

    ret = flash_get_runtime_config(&updated_config);
    if (ret != ESP_OK) return ret;

    // Generate field map automatically using macro
    field_map_t fields[] = {
        #define DATA(name, type, default_val) {#name, #type, offsetof(data_config_t, name), sizeof(((data_config_t*)0)->name)},
        #define DATA_ARRAY(name, type, size, default_val) {#name, #type"["#size"]", offsetof(data_config_t, name), sizeof(((data_config_t*)0)->name)},
        CONFIG_FIELDS
        #undef DATA
        #undef DATA_ARRAY
    };

    size_t n = sizeof(fields) / sizeof(fields[0]);
    for (size_t i = 0; i < n; i++) {
        if (strcmp(fields[i].name, field_name) == 0) {
            ret = update_field(&updated_config, fields[i], value);
            if (ret != ESP_OK) return ret;
            return flash_edit_config(updated_config);
        }
    }

    return ESP_ERR_INVALID_ARG;
}

// |--- Type parsing functions ---|

static esp_err_t parse_int32_t(const char *value, int32_t *out) {
    if (!value || !out) return ESP_ERR_INVALID_ARG;
    char *endptr = NULL;
    long val = strtol(value, &endptr, 10);
    if (*endptr != '\0' || val < INT32_MIN || val > INT32_MAX) return ESP_ERR_INVALID_ARG;
    *out = (int32_t)val;
    return ESP_OK;
}

static esp_err_t parse_uint8_t(const char *value, uint8_t *out) {
    if (!value || !out) return ESP_ERR_INVALID_ARG;
    char *endptr = NULL;
    unsigned long val = strtoul(value, &endptr, 10);
    if (*endptr != '\0' || val > UINT8_MAX) return ESP_ERR_INVALID_ARG;
    *out = (uint8_t)val;
    return ESP_OK;
}

static esp_err_t parse_float(const char *value, float *out) {
    if (!value || !out) return ESP_ERR_INVALID_ARG;
    char *endptr = NULL;
    *out = strtof(value, &endptr);
    if (*endptr != '\0') return ESP_ERR_INVALID_ARG;
    return ESP_OK;
}

static esp_err_t parse_double(const char *value, double *out) {
    if (!value || !out) return ESP_ERR_INVALID_ARG;
    char *endptr = NULL;
    *out = strtod(value, &endptr);
    if (*endptr != '\0') return ESP_ERR_INVALID_ARG;
    return ESP_OK;
}

static esp_err_t parse_char(const char *value, char *out) {
    if (!value || !out || strlen(value) != 1) return ESP_ERR_INVALID_ARG;
    *out = value[0];
    return ESP_OK;
}

static esp_err_t parse_string(const char *value, char *out, size_t max_size) {
    if (!value || !out) return ESP_ERR_INVALID_ARG;
    size_t copy_size = (strlen(value) < max_size - 1) ? strlen(value) : max_size - 1;
    memcpy(out, value, copy_size);
    out[copy_size] = '\0';
    return ESP_OK;
}