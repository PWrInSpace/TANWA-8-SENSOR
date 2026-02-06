///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 27.01.2024 by Michał Kos
/// Updated: 06.02.2026 by Mateusz Kluczka
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains declaration of the system console configuration, including initialization
/// and available commands for debugging/testing purposes.
///===-----------------------------------------------------------------------------------------===//
#ifndef PWRINSPACE_CONSOLE_CONFIG_H_
#define PWRINSPACE_CONSOLE_CONFIG_H_

#include "flash.h"
#include "console.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "argtable3/argtable3.h"

#include "BoardData.h"

/**
 * @brief Initialize console configuration (CLI settings, commands, etc.).
 * @return `ESP_OK` if successful, error code if failed
 */
esp_err_t console_config_init(void);

#endif // PWRINSPACE_CONSOLE_CONFIG_H_