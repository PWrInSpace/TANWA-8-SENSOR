#ifndef PWRINSPACE_CONSOLE_H_
#define PWRINSPACE_CONSOLE_H_

#include "argtable3/argtable3.h"
#include "driver/uart.h"
#include "driver/uart_vfs.h"
#include "esp_console.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "linenoise/linenoise.h"
#include <string.h>

// For some serial monitors this lib is crashing
// Works with cutecom ...
// Print giga good message to default stream
#define CONSOLE_WRITE_G(format, ...) printf(" # G: " format "\n", ##__VA_ARGS__)

// Print error message to default stream
#define CONSOLE_WRITE_E(format, ...) printf(" # E: " format "\n", ##__VA_ARGS__)

// Print to default stream
#define CONSOLE_WRITE(format, ...) printf(" # " format "\n", ##__VA_ARGS__)

#define PREFIX "ESP>"

typedef void (*console_completion_fn_t)(const char *buf,
                                        linenoiseCompletions *lc);
typedef struct {
  esp_console_cmd_t cmd;
  console_completion_fn_t arg_completion;
} console_cmd_ex_t;

typedef struct {
  const char *prefix;
  size_t prefix_len;
  const char *token;
  size_t token_len;
} cli_split_t;

/**
 * @brief Initialize the console subsystem.
 * @return `ESP_OK` if successful, `ESP_FAIL` if failed
 */
esp_err_t console_init(void);

/**
 * @brief Find a registered console command by name.
 * @param name [in] command name
 * @return pointer to the command structure, or `NULL` if not found
 */
esp_console_cmd_t *find_cmd_by_name(const char *name);

/**
 * @brief Print usage information for a console command.
 * @param name [in] command name
 * @return `ESP_OK` if successful, error code if failed
 */
esp_err_t print_cmd_usage(const char *name);

/**
 * @brief Split the last token from a CLI line.
 * @param line [in] command line
 * @return structure containing the last token and remaining line
 */
cli_split_t cli_split_last_token(const char *line);

/**
 * @brief Register multiple console commands at once.
 * @param commands [in] array of command structures
 * @param number_of_cmd [in] number of commands in the array
 * @return `ESP_OK` if successful, error code if failed
 */
esp_err_t console_register_commands(console_cmd_ex_t *commands,
                                    size_t number_of_cmd);

/**
 * @brief Deinitialize the console subsystem.
 * @return `ESP_OK` if successful, `ESP_ERR_INVALID_STATE` if not initialized
 * yet
 */
esp_err_t console_deinit(void);

#endif // PWRINSPACE_CONSOLE_H