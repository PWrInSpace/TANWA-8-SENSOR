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

const char **flash_get_field_names(size_t *count) {
    enum {
        FIELDS_COUNT = 0
        #define DATA(...)             + 1
        #define DATA_ARRAY(...)       + 1
        #define SECTION_BEGIN(...)
        #define SECTION_END(...)
        CONFIG_FIELDS
        #undef DATA
        #undef DATA_ARRAY
        #undef SECTION_BEGIN
        #undef SECTION_END
    };

    static const char *names[FIELDS_COUNT];
    static int initialized = 0;

    if (!initialized) {
        size_t reg_count = 0;
        field_map_t *nodes = flash_build_registry(&reg_count);

        for (size_t i = 0; i < reg_count; i++) names[i] = nodes[i].name;
        initialized = 1;
    }

    if (count) *count = FIELDS_COUNT;
    return names;
}

typedef struct {
    const char *name;
    const char *type;
    size_t offset;
    size_t field_size;
} field_map_t;

field_map_t *flash_build_registry(size_t *out) {
    enum {
        TOTAL_SECTION_CHARS = 0
        #define DATA(...)
        #define DATA_ARRAY(...)
        #define SECTION_BEGIN(name)   + sizeof(#name)
        #define SECTION_END(...)
        CONFIG_FIELDS
        #undef DATA
        #undef DATA_ARRAY
        #undef SECTION_BEGIN
        #undef SECTION_END
    };

    static char name_pool[2048]; //TODO
    static field_map_t nodes[] = {
        #define DATA(name, type, default) { #name, #type, 0, sizeof(type) },
        #define DATA_ARRAY(name, type, size, default) DATA(name, type[size], default)
        #define SECTION_BEGIN(...)
        #define SECTION_END(...)

        CONFIG_FIELDS
        
        #undef DATA
        #undef DATA_ARRAY
        #undef SECTION_BEGIN
        #undef SECTION_END
    };

    static int initialized = 0;
    if (!initialized) {
        int i = 0;
        const char *prefix = "";
        char *pool_ptr = name_pool;
        data_config_t dummy_instance;
        data_config_t *ctx = &dummy_instance;

        #define DATA(name, type, default) \
            nodes[i].offset = (char*)&ctx->name - (char*)&dummy_instance; \
            nodes[i].name = pool_ptr; \
            if (prefix[0] == '\0') pool_ptr += sprintf(pool_ptr, "%s", #name) + 1; \
            else pool_ptr += sprintf(pool_ptr, "%s.%s", prefix, #name) + 1; \
            i++;
        #define DATA_ARRAY(name, type, size, default) DATA(name, type[size], default)
        #define SECTION_BEGIN(name) { \
            /* offset calculation*/ \
            typeof(ctx->name) *next_ptr = &ctx->name; \
            typeof(next_ptr) ctx = next_ptr; \
            /* full name creation */ \
            size_t _len = strlen(prefix) + sizeof(#name) + 1; /* (+1) for string termination */ \
            char _new_path[_len]; \
            if (prefix[0] == '\0') sprintf(_new_path, "%s", #name); \
            else sprintf(_new_path, "%s.%s", prefix, #name); \
            const char *prefix = _new_path;
        #define SECTION_END(name) } 

        CONFIG_FIELDS

        #undef SECTION_BEGIN
        #undef SECTION_END
        #undef DATA
        #undef DATA_ARRAY

        initialized = 1;
    }

    if (out) *out = sizeof(nodes) / sizeof(nodes[0]);
    return nodes;
}

static esp_err_t parse_int32_t(const char *value, int32_t *out);
static esp_err_t parse_uint8_t(const char *value, uint8_t *out);
static esp_err_t parse_float(const char *value, float *out);
static esp_err_t parse_double(const char *value, double *out);
static esp_err_t parse_char(const char *value, char *out);
static esp_err_t parse_string(const char *value, char *out, size_t max_size);

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

esp_err_t flash_edit_field(const char *field_name, const char *value) {
    if (!value || !field_name) return ESP_ERR_INVALID_ARG;

    esp_err_t ret;
    data_config_t updated_config;

    ret = flash_get_runtime_config(&updated_config);
    if (ret != ESP_OK) return ret;

    size_t n = 0;
    field_map_t *registry = flash_build_registry(&n);

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