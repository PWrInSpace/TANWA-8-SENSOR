///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 06.05.2025 by Szymon Rzewuski
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains the definitions of functions for the setup task task
///===-----------------------------------------------------------------------------------------===//

#ifndef PWRINSPACE_SETUP_TASK_H
#define PWRINSPACE_SETUP_TASK_H

#include "esp_err.h"

void setup_task(void *arg);
esp_err_t setup_task_init(void);
esp_err_t setup_task_deinit(void);

#endif //PWRINSPACE_CAN_TASK_H