#pragma once
#include "sdkconfig.h"

// Bridge CONFIG_ flags to UM_BOARD_* so you can also -DUM_BOARD_xxx if preferred
#if defined(CONFIG_UM_BOARD_TINYS3D) && !defined(UM_BOARD_TINYS3D)
#define UM_BOARD_TINYS3D
#elif defined(CONFIG_UM_BOARD_FEATHERS3D) && !defined(UM_BOARD_FEATHERS3D)
#define UM_BOARD_FEATHERS3D
#elif defined(CONFIG_UM_BOARD_PROS3D) && !defined(UM_BOARD_PROS3D)
#define UM_BOARD_PROS3D
#elif defined(CONFIG_UM_BOARD_EDGES3D) && !defined(UM_BOARD_EDGES3D)
#define UM_BOARD_EDGES3D
#elif defined(CONFIG_UM_BOARD_TINYPICOD) && !defined(UM_BOARD_TINYPICOD)
#define UM_BOARD_TINYPICOD
#elif defined(CONFIG_UM_BOARD_TINYC6D) && !defined(UM_BOARD_TINYC6D)
#define UM_BOARD_TINYC6D
#endif

// ---------- Board-fixed pin maps (from the Arduino helper & board schematics) ----------
// -1 means the board doesn't have that feature
#if defined(UM_BOARD_TINYS3D)
#define UM_BOARD_NAME "TinyS3[D]"
#define UM_GPIO_PIXEL 18     // RGB_DATA
#define UM_GPIO_PIXEL_PWR 17 // RGB_PWR
#define UM_GPIO_LDO2 -1
#define UM_GPIO_VBUS_SENSE 33
#define UM_GPIO_BLUE_LED -1
#define UM_GPIO_RF_SWITCH 38 // High = external antenna
#define UM_GPIO_ALS -1
#define UM_GPIO_I2C_SDA 8
#define UM_GPIO_I2C_SCL 9

#elif defined(UM_BOARD_FEATHERS3D)
#define UM_BOARD_NAME "FeatherS3[D]"
#define UM_GPIO_PIXEL 40     // RGB_DATA
#define UM_GPIO_PIXEL_PWR 39 // RGB_PWR is LDO2
#define UM_GPIO_LDO2 39
#define UM_GPIO_VBUS_SENSE 34
#define UM_GPIO_BLUE_LED 13 // LED_BUILTIN
#define UM_GPIO_RF_SWITCH 41
#define UM_GPIO_ALS 4 // ALS on ADC1 (GPIO4)
#define UM_GPIO_I2C_SDA 8
#define UM_GPIO_I2C_SCL 9

#elif defined(UM_BOARD_PROS3D)
#define UM_BOARD_NAME "ProS3[D]"
#define UM_GPIO_PIXEL 18
#define UM_GPIO_PIXEL_PWR 17 // RGB_PWR is LDO2
#define UM_GPIO_LDO2 17
#define UM_GPIO_VBUS_SENSE 33
#define UM_GPIO_BLUE_LED -1
#define UM_GPIO_RF_SWITCH 11
#define UM_GPIO_ALS -1
#define UM_GPIO_I2C_SDA 8
#define UM_GPIO_I2C_SCL 9

#elif defined(UM_BOARD_EDGES3D)
#define UM_BOARD_NAME "EdgeS3[D]"
#define UM_GPIO_PIXEL -1
#define UM_GPIO_PIXEL_PWR -1
#define UM_GPIO_LDO2 -1
#define UM_GPIO_VBUS_SENSE -1
#define UM_GPIO_BLUE_LED -1
#define UM_GPIO_RF_SWITCH 38
#define UM_GPIO_ALS -1
#define UM_GPIO_I2C_SDA 8
#define UM_GPIO_I2C_SCL 9

#elif defined(UM_BOARD_TINYPICOD)
#define UM_BOARD_NAME "TinyPICO[D]"
#define UM_GPIO_PIXEL 2
#define UM_GPIO_PIXEL_PWR 13
#define UM_GPIO_LDO2 -1
#define UM_GPIO_VBUS_SENSE 9
#define UM_GPIO_BLUE_LED -1
#define UM_GPIO_RF_SWITCH 12
#define UM_GPIO_ALS -1
#define UM_GPIO_I2C_SDA 21
#define UM_GPIO_I2C_SCL 22

#elif defined(UM_BOARD_TINYC6D)
#define UM_BOARD_NAME "TinyC6[D]"
#define UM_GPIO_PIXEL 23
#define UM_GPIO_PIXEL_PWR 22
#define UM_GPIO_LDO2 -1
#define UM_GPIO_VBUS_SENSE 5
#define UM_GPIO_BLUE_LED -1
#define UM_GPIO_RF_SWITCH -1 // RF switch is on the IO expander
#define UM_GPIO_ALS -1
#define UM_GPIO_I2C_SDA 6
#define UM_GPIO_I2C_SCL 7
#define UM_HAS_IO_EXPANDER 1
#define UM_IOX_RF_SWITCH 0 // FXL6408 XIO0
#if defined(CONFIG_UM_IOX_I2C_ADDR)
#define UM_IOX_I2C_ADDR CONFIG_UM_IOX_I2C_ADDR
#else
#define UM_IOX_I2C_ADDR 0x43
#endif

#else
#error "No UM_BOARD_xxx defined. Select a board in menuconfig or pass -DUM_BOARD_TINYS3D (etc.)."
#endif

// Catch a board that doesn't match the IDF target (menuconfig already enforces this)
#if (defined(UM_BOARD_TINYS3D) || defined(UM_BOARD_FEATHERS3D) || defined(UM_BOARD_PROS3D) || defined(UM_BOARD_EDGES3D)) && !CONFIG_IDF_TARGET_ESP32S3
#error "This Series[D] board needs the esp32s3 target (idf.py set-target esp32s3)"
#elif defined(UM_BOARD_TINYPICOD) && !CONFIG_IDF_TARGET_ESP32
#error "TinyPICO[D] needs the esp32 target (idf.py set-target esp32)"
#elif defined(UM_BOARD_TINYC6D) && !CONFIG_IDF_TARGET_ESP32C6
#error "TinyC6[D] needs the esp32c6 target (idf.py set-target esp32c6)"
#endif
