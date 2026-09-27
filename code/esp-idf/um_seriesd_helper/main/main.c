#include "um_seriesd.h"
#include "um_seriesd_pins.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ESP_ERROR_CHECK(um_begin());
    ESP_LOGI("app", "Board: %s", um_board_name());

    // Pixel demo (setting a color turns the pixel power on)
    um_set_pixel_brightness(32);
    for (int i = 0; i < 255; i += 8)
    {
        um_set_pixel_color_u32(um_color_wheel(i));
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    um_set_pixel_power(false);

    // ALS (FeatherS3[D] only)
    float v = 0.0f;
    if (um_get_light_sensor_voltage(&v) == ESP_OK)
    {
        ESP_LOGI("app", "ALS = %.3f V", v);
    }

    // VBUS
    ESP_LOGI("app", "VBUS: %s", um_get_vbus_present() ? "present" : "not present");

    // Fuel gauge on the board's I2C pins @ 400kHz
    // (on TinyC6[D] um_begin() already set up this bus for the IO expander, so it's reused)
    if (um_fg_setup(0, UM_GPIO_I2C_SDA, UM_GPIO_I2C_SCL, 400000) == ESP_OK)
    {
        float vbatt = 0.0f;
        if (um_fg_get_battery_voltage(&vbatt) == ESP_OK)
            ESP_LOGI("app", "VBAT = %.3f V", vbatt);
        uint8_t ver = 0;
        if (um_fg_get_version(&ver) == ESP_OK)
            ESP_LOGI("app", "MAX17048 version = 0x%02X", ver);
    }

    // RF switch (on the IO expander on TinyC6[D])
    um_set_antenna_external(true);
    vTaskDelay(pdMS_TO_TICKS(300));
    um_set_antenna_external(false);
}
