///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 27.01.2024 by Michał Kos
/// Updated: 06.02.2026 by Mateusz Kluczka
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation of the system console configuration, including initialization
/// and available commands for debugging/testing purposes.
///===-----------------------------------------------------------------------------------------===//
#include "console_config.h"

#define TAG "CONSOLE_CONFIG"

// |--- Miscellaneous comands ---|

int restart_device(int argc, char **argv) {
    ESP_LOGI(TAG, "Restarting device...");
    esp_restart();
    return 0;
}

static int read_temperature(int argc, char **argv) {
    uint8_t ret = 0;
    float temp;
    temp = BoardData.status_temp;
    CONSOLE_WRITE("TMP1075 Temperature:");
    CONSOLE_WRITE("#1 => temp1 = %f", temp);

    return 0;
}

// |--- Commands for Flash memory module ---|

static struct {
    struct arg_str *field;
    struct arg_str *value;
    struct arg_end *end;
} edit_flash_args;

static struct {
    struct arg_str *confirmation;
    struct arg_end *end;
} erase_flash_args;

void print_config(const data_config_t *cfg, const char *label) {
    printf("%s\n", label);
    flash_print_config(*cfg);
    printf("\n");
}

int read_flash(int argc, char **argv) {
    data_config_t data;
    if (flash_read(&data) != ESP_OK) {
        printf("Couldn't retrieve data from flash memory\n");
        return 0;
    }

    print_config(&data, "Memory contents:");
    return 0;
}

int get_runtime_config(int argc, char **argv) {
    data_config_t data;
    flash_get_runtime_config(&data);
    print_config(&data, "Runtime config:");
    return 0;
}

int save_flash(int argc, char **argv) {
    esp_err_t ret;
    ret = flash_commit();

    if (ret != ESP_OK) {
        printf("Couldn't save data to flash memory\nErr: %s\n", esp_err_to_name(ret));
        return 0;
    }

    printf("Successfully saved data to flash memory\n");
    return 0;
}

int restore_defaults(int argc, char **argv) {
    esp_err_t ret;
    ret = flash_restore_defaults();

    if (ret != ESP_OK) {
        printf("Couldn't restore config default values\nErr: %s\n", esp_err_to_name(ret));
        return 0;
    }

    printf("Successfully restored config default values. Remember to use `save_config` to save your changes\n");
    return 0;
}

int erase_flash(int argc, char **argv) {
    if (argc < 2) {
        print_cmd_usage(argv[0]);
        return 0;
    }
    const char *confirmation = NULL;

    // Attempt to parse argument
    int nerrors = arg_parse(argc, argv, (void **)&erase_flash_args);
    if (nerrors == 0) {
        confirmation = erase_flash_args.confirmation->sval[0];
    } else if (argc == 2) {
        confirmation = argv[1];
    }

    if (!confirmation) {
        arg_print_errors(stdout, erase_flash_args.end, argv[0]);
        return 0;
    }

    if (strcmp(confirmation, "Y") != 0) {
        printf("Flash erase cancelled. You need to pass 'Y' as argument to confirm.\n");
        return 0;
    }

    esp_err_t ret = flash_erase_config();
    if (ret != ESP_OK) {
        printf("Couldn't erase flash contents\nErr: %s\n", esp_err_to_name(ret));
        return 0;
    }

    printf("Successfully erased flash contents\n");
    return 0;
}

int edit_flash(int argc, char **argv) {
    if (argc < 3) {
        print_cmd_usage(argv[0]);
        return 0;
    }

    const char *field = NULL;
    const char *value = NULL;

    // Attempt to parse arguments
    int nerrors = arg_parse(argc, argv, (void **)&edit_flash_args);

    if (nerrors == 0) {
        field = edit_flash_args.field->sval[0];
        value = edit_flash_args.value->sval[0];
    } else if (argc == 3) {
        field = argv[1];
        value = argv[2];
    }

    if (!field || !value) {
        arg_print_errors(stdout, edit_flash_args.end, argv[0]);
        return 0;
    }
    
    esp_err_t ret = flash_edit_field(field, value);
    if (ret != ESP_OK) {
        printf("Couldn't edit provided field\nErr: %s\n", esp_err_to_name(ret));
        return 0;
    }

    printf("Successfully edited runtime config. Remember to use `save_config` to save your changes\n");
    return 0;
}

void edit_flash_completion(const char *buf, linenoiseCompletions *lc) {
    if (!buf || !lc) return;

    // get list of all possible argument strings
    size_t n = 0;
    const char **fields = flash_get_field_names(&n);
    if (!fields || n == 0) return;

    cli_split_t split = cli_split_last_token(buf);

    for (size_t i = 0; i < n; i++) {
        // check if argument string can qualify as completion
        if (split.token_len < strlen(fields[i]) && strncmp(split.token, fields[i], split.token_len) == 0) {
            size_t len = split.prefix_len + strlen(fields[i]) + 1;
            char *completion = malloc(len);
            snprintf(completion, len, "%.*s%s", split.prefix_len, split.prefix, fields[i]);

            linenoiseAddCompletion(lc, completion);
            free(completion);
        }
    }
}

static esp_err_t setup_commands(int *cmd_count, console_cmd_ex_t **cmd_list) {
    // setup argtables for commands that need them
    edit_flash_args.field = arg_str1("f", "field", "<string>", "Name of field to edit");
    edit_flash_args.value = arg_str1("v", "value", "<string>", "New value for the field");
    edit_flash_args.end = arg_end(2);

    erase_flash_args.confirmation = arg_str1("c", "confirm", "<Y|N>", "Confirmation, so nobody will accidentally erase data stored in flash memory");
    erase_flash_args.end = arg_end(1);

    // setup all commands
    static console_cmd_ex_t cmd[] = {
        {
            .cmd = {
                .command  = "restart",
                .help     = "Restarts this device.",
                .hint     = NULL,
                .func     = restart_device
            }
        },
        {
            .cmd = {
                .command  = "temp-read",
                .help     = "Read temperature from BoardData.",
                .hint     = NULL,
                .func     = read_temperature
            }
        },
        {
            .cmd = {
                .command  = "read_flash",
                .help     = "Reads and displays saved data in flash memory.",
                .hint     = NULL,
                .func     = read_flash
            }
        },
        {
            .cmd = {
                .command  = "display_config",
                .help     = "Displays current state of runtime config.\nThis command does NOT display flash memory contents, to see current flash memory contents use `read_flash`.",
                .hint     = NULL,
                .func     = get_runtime_config
            }
        },
        {
            .cmd = {
                .command  = "save_config",
                .help     = "Saves runtime config edited by User to flash memory.",
                .hint     = NULL,
                .func     = save_flash
            }
        },
        {
            .cmd = {
                .command  = "edit_config",
                .help     = "Sets the provided field in runtime config to provided value.",
                .hint     = NULL,
                .func     = edit_flash,
                .argtable = &edit_flash_args
            },
            .arg_completion = edit_flash_completion
        },
        {
            .cmd = {
                .command  = "restore_config",
                .help     = "Restores all default values and saves them into runtime config.\nUse `save_config` to save the runtime config to flash memory.",
                .hint     = NULL,
                .func     = restore_defaults
            }
        },
        {
            .cmd = {
                .command  = "erase_flash",
                .help     = "Erases flash memory partition that is holding config data.\nTo erase stored data you need to type `erase_flash Y` to ensure that flash won't be erased by accident.\nThere is no need to run `save_flash` after this function finishes.",
                .hint     = NULL,
                .func     = erase_flash,
                .argtable = &erase_flash_args
            }
        }
    };


    *cmd_count = sizeof(cmd) / sizeof(cmd[0]);
    *cmd_list = cmd;
    return ESP_OK;
}


esp_err_t console_config_init(void) {
    esp_err_t ret;
    ret = console_init();
    if (ret != ESP_OK) {
        return ret;
    }

    int cmd_count;
    console_cmd_ex_t *cmd_list;

    ret = setup_commands(&cmd_count, &cmd_list);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = console_register_commands(cmd_list, cmd_count);
    return ret;
}