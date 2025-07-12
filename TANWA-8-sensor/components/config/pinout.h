/**
 * @file pinout.h
 * @author Michał Kos
 * @brief A header file with pinout description
 * @version 0.1
 * @date 2023-03-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef PINOUT_HH
#define PINOUT_HH

// Boot
#define BOOT_BUTTON 0

// Leds
#define ESP_LED 5U
#define STATUS_LED 2U

// SPI
#define SPI_MISO 19U
#define SPI_MOSI 23U
#define SPI_SCK 18U

// UART
#define UART2_RX 16U
#define UART2_TX 17U    

// Thermocouples
#define THERMOCOUPLE_CS1 16U
#define THERMOCOUPLE_CS2 15U
#define THERMOCOUPLE_CS3 7U

#endif // PINOUT_HH