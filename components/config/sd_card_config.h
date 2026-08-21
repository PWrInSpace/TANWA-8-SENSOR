#ifndef SD_CARD_CONFIG
#define SD_CARD_CONFIG

#include <stdbool.h>
#include <stdint.h>

#define SD_CS_Pin 38 // TODO: Check pin

/**
 * @brief Initalize sd card task
 *
 * @return true :D
 * @return false :C
 */
bool initialize_sd(void);

#endif