#ifndef PINOUT_HH
#define PINOUT_HH

// Boot
#define BOOT_BUTTON 0

// Leds
#define ESP_LED 5U
#define STATUS_LED 2U

// SPI SD
#define SPI_SD_MISO 41
#define SPI_SD_MOSI 39
#define SPI_SD_SCK 40
#define SPI_SD_CS 38

// SPI Thermocouples
#define SPI_TC_SCK 9
#define SPI_TC_MOSI 11
#define SPI_TC_MISO 10

// UART
#define UART2_RX 16U
#define UART2_TX 17U

// Thermocouples CS
#define THERMOCOUPLE_CS1 16U
#define THERMOCOUPLE_CS2 15U
#define THERMOCOUPLE_CS3 7U

#endif // PINOUT_HH
