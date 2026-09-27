#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // ----- Lifecycle -----
    esp_err_t um_begin(void); // init GPIO/RMT/ADC (and the I2C bus + IO expander on TinyC6[D])
    void um_end(void);

    // ----- Power / GPIO -----
    void um_set_ldo2_power(bool on); // ProS3[D] and FeatherS3[D] only
    void um_set_pixel_power(bool on); // On ProS3[D] and FeatherS3[D] this is LDO2

    // ----- Pixel (WS2812, GRB) - not on EdgeS3[D] -----
    // Setting a color turns the pixel power on
    void um_set_pixel_brightness(uint8_t b); // 0..255
    void um_set_pixel_color_rgb(uint8_t r, uint8_t g, uint8_t b);
    void um_set_pixel_color_u32(uint32_t rgb);
    uint32_t um_color(uint8_t r, uint8_t g, uint8_t b);
    uint32_t um_color_wheel(uint8_t pos);

    // ----- Blue LED (FeatherS3[D] only) -----
    void um_set_blue_led(bool on);
    void um_toggle_blue_led(void);

    // ----- Measurements -----
    esp_err_t um_get_light_sensor_voltage(float *volts); // FeatherS3[D] only; volts
    bool um_get_vbus_present(void);                      // Not on EdgeS3[D]

    // ----- I2C bus (shared by the fuel gauge and IO expander) -----
    // um_fg_setup() creates the bus. On TinyC6[D] um_begin() already creates it on
    // the board's I2C pins, in which case um_fg_setup() reuses that bus.
    esp_err_t um_fg_setup(int i2c_port, int gpio_sda, int gpio_scl, uint32_t hz);
    // Use an I2C master bus your app already created instead (on TinyC6[D], call before um_begin())
    esp_err_t um_fg_setup_bus(i2c_master_bus_handle_t bus);
    // The bus in use (NULL until one is set up), to add your own devices to it
    i2c_master_bus_handle_t um_get_i2c_bus(void);

    // ----- Fuel Gauge (MAX17048, I2C) -----
    esp_err_t um_fg_get_battery_voltage(float *volts); // VCELL * 78.125uV
    esp_err_t um_fg_get_version(uint8_t *ver);

    // ----- IO expander (FXL6408, I2C) - TinyC6[D] only -----
    // XIO0 is the RF switch; um_begin() sets it up. Others return ESP_ERR_NOT_SUPPORTED.
    typedef enum
    {
        UM_IOX_INPUT,
        UM_IOX_OUTPUT,
        UM_IOX_INPUT_PULLUP,
        UM_IOX_INPUT_PULLDOWN,
    } um_iox_mode_t;

    esp_err_t um_iox_begin(void); // ESP_ERR_NOT_FOUND if the FXL6408 doesn't answer
    esp_err_t um_iox_pin_mode(uint8_t pin, um_iox_mode_t mode);
    esp_err_t um_iox_digital_write(uint8_t pin, bool level);
    esp_err_t um_iox_digital_read(uint8_t pin, bool *level);

    // ----- RF Switch (Series[D]): high = external antenna -----
    // On TinyC6[D] the RF switch is on the IO expander (XIO0)
    void um_set_antenna_external(bool external);

    // ----- Info -----
    const char *um_board_name(void);

#ifdef __cplusplus
}
#endif
