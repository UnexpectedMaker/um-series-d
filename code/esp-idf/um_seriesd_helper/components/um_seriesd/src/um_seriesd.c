#include "um_seriesd.h"
#include "um_seriesd_pins.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_rom_sys.h"
#include "driver/gpio.h"
#if UM_GPIO_ALS >= 0
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#endif

static const char *TAG = "um_seriesd";

#define UM_NOT_AVAILABLE() ESP_LOGW(TAG, "%s not available on %s", __func__, UM_BOARD_NAME)

// --- WS2812 backend ---
esp_err_t um_ws_init(void);
void um_ws_deinit(void);
void um_ws_set_brightness(uint8_t b);
esp_err_t um_ws_show_rgb(uint8_t r, uint8_t g, uint8_t b);
esp_err_t um_ws_refresh(void);

static bool s_pixel_pwr = false;
static bool s_blue_led = false;

#if UM_GPIO_ALS >= 0
// --- ADC state (ALS only; VBUS is digital) ---
static adc_oneshot_unit_handle_t s_adc_als = NULL;
static adc_cali_handle_t s_cali_als = NULL;
static adc_channel_t s_adc_als_channel;
#endif

// --- I2C bus, shared by the Fuel Gauge (MAX17048 @ 0x36) and IO expander ---
#define UM_I2C_PORT 0
#define UM_I2C_HZ 400000
#define UM_I2C_TIMEOUT_MS 100
#define UM_FG_ADDR 0x36
static i2c_master_bus_handle_t s_bus = NULL;
static bool s_bus_owned = false; // true when we created the bus, so um_end() deletes it
static uint32_t s_i2c_hz = UM_I2C_HZ;
static i2c_master_dev_handle_t s_fg_dev = NULL;

#if defined(UM_HAS_IO_EXPANDER)
// --- IO expander (FXL6408) ---
enum
{
    FXL6408_DEVICE_ID = 0x01,
    FXL6408_IO_DIR = 0x03,
    FXL6408_OUTPUT_STATE = 0x05,
    FXL6408_OUTPUT_HIGH_Z = 0x07,
    FXL6408_INPUT_DEFAULT_STATE = 0x09,
    FXL6408_PULL_ENABLE = 0x0B,
    FXL6408_PULL_UP_DOWN = 0x0D,
    FXL6408_INPUT_STATUS = 0x0F,
    FXL6408_INT_MASK = 0x11,
    FXL6408_INT_STATUS = 0x13,
};
static i2c_master_dev_handle_t s_iox_dev = NULL;
#endif

static void cfg_output_if_valid(int gpio, int level)
{
    if (gpio >= 0)
    {
        gpio_config_t io = {.pin_bit_mask = 1ULL << gpio, .mode = GPIO_MODE_OUTPUT, .pull_up_en = 0, .pull_down_en = 0, .intr_type = GPIO_INTR_DISABLE};
        gpio_config(&io);
        gpio_set_level(gpio, level);
    }
}
static void cfg_input_if_valid(int gpio)
{
    if (gpio >= 0)
    {
        // No pulls - VBUS sense is driven by a divider
        gpio_config_t io = {.pin_bit_mask = 1ULL << gpio, .mode = GPIO_MODE_INPUT, .pull_up_en = 0, .pull_down_en = 0, .intr_type = GPIO_INTR_DISABLE};
        gpio_config(&io);
    }
}

// --- I2C helpers ---
static esp_err_t i2c_new_bus(int i2c_port, int gpio_sda, int gpio_scl, uint32_t hz)
{
    if (s_bus)
        return ESP_OK; // already set up (by um_begin() on TinyC6[D], or an earlier call)
    i2c_master_bus_config_t cfg = {
        .i2c_port = i2c_port,
        .sda_io_num = gpio_sda,
        .scl_io_num = gpio_scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true};
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&cfg, &s_bus), TAG, "i2c bus");
    s_bus_owned = true;
    s_i2c_hz = hz;
    return ESP_OK;
}

static esp_err_t i2c_add_dev(uint16_t addr, i2c_master_dev_handle_t *dev)
{
    if (*dev)
        return ESP_OK;
    if (!s_bus)
        return ESP_ERR_INVALID_STATE;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = s_i2c_hz};
    return i2c_master_bus_add_device(s_bus, &cfg, dev);
}

// --- Public: lifecycle ---
esp_err_t um_begin(void)
{
    ESP_LOGI(TAG, "Init %s", UM_BOARD_NAME);

    // Pixel power / LDO2 off by default
    cfg_output_if_valid(UM_GPIO_PIXEL_PWR, 0);
    s_pixel_pwr = false;

    // WS2812
    ESP_RETURN_ON_ERROR(um_ws_init(), TAG, "ws2812 init");
    um_ws_set_brightness(CONFIG_UM_PIXEL_DEFAULT_BRIGHTNESS);

    // Blue LED default OFF
    cfg_output_if_valid(UM_GPIO_BLUE_LED, 0);
    s_blue_led = false;

    // VBUS input
    cfg_input_if_valid(UM_GPIO_VBUS_SENSE);

    // RF switch default to onboard antenna (low)
#if defined(UM_HAS_IO_EXPANDER)
    // The IO expander is needed from um_begin(), so this sets up the I2C bus on the
    // board's pins if um_fg_setup()/um_fg_setup_bus() haven't been called yet
    if (um_iox_begin() == ESP_OK)
    {
        um_iox_digital_write(UM_IOX_RF_SWITCH, false);
        um_iox_pin_mode(UM_IOX_RF_SWITCH, UM_IOX_OUTPUT);
    }
    else
    {
        ESP_LOGW(TAG, "IO expander not found - RF switch unavailable");
    }
#else
    cfg_output_if_valid(UM_GPIO_RF_SWITCH, 0);
#endif

#if UM_GPIO_ALS >= 0
    // ALS (FeatherS3[D]): set up ADC
    adc_unit_t unit;
    if (adc_oneshot_io_to_channel(UM_GPIO_ALS, &unit, &s_adc_als_channel) == ESP_OK)
    {
        adc_oneshot_unit_init_cfg_t ucfg = {.unit_id = unit, .ulp_mode = ADC_ULP_MODE_DISABLE};
        ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&ucfg, &s_adc_als), TAG, "adc unit");
        adc_oneshot_chan_cfg_t ccfg = {.bitwidth = ADC_BITWIDTH_DEFAULT, .atten = ADC_ATTEN_DB_12};
        ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(s_adc_als, s_adc_als_channel, &ccfg), TAG, "adc ch");

        adc_cali_curve_fitting_config_t cal = {.unit_id = unit, .chan = s_adc_als_channel, .atten = ccfg.atten, .bitwidth = ADC_BITWIDTH_DEFAULT};
        if (adc_cali_create_scheme_curve_fitting(&cal, &s_cali_als) != ESP_OK)
            s_cali_als = NULL;
    }
    else
    {
        ESP_LOGW(TAG, "ALS gpio->adc mapping failed");
    }
#endif

    return ESP_OK;
}

void um_end(void)
{
    um_ws_deinit();
#if UM_GPIO_ALS >= 0
    if (s_adc_als)
    {
        adc_oneshot_del_unit(s_adc_als);
        s_adc_als = NULL;
    }
    if (s_cali_als)
    {
        adc_cali_delete_scheme_curve_fitting(s_cali_als);
        s_cali_als = NULL;
    }
#endif
#if defined(UM_HAS_IO_EXPANDER)
    if (s_iox_dev)
    {
        i2c_master_bus_rm_device(s_iox_dev);
        s_iox_dev = NULL;
    }
#endif
    if (s_fg_dev)
    {
        i2c_master_bus_rm_device(s_fg_dev);
        s_fg_dev = NULL;
    }
    if (s_bus && s_bus_owned)
        i2c_del_master_bus(s_bus);
    s_bus = NULL;
    s_bus_owned = false;
}

// --- Power / LDO2 ---
void um_set_ldo2_power(bool on)
{
#if UM_GPIO_LDO2 >= 0
    gpio_set_level(UM_GPIO_LDO2, on ? 1 : 0);
    s_pixel_pwr = on; // LDO2 also powers the pixel
#else
    UM_NOT_AVAILABLE();
#endif
}

void um_set_pixel_power(bool on)
{
#if UM_GPIO_PIXEL_PWR >= 0
    gpio_set_level(UM_GPIO_PIXEL_PWR, on ? 1 : 0);
    s_pixel_pwr = on;
#else
    UM_NOT_AVAILABLE();
#endif
}

// --- Pixel API ---
#if UM_GPIO_PIXEL >= 0
static void pixel_power_on(void)
{
    if (!s_pixel_pwr)
    {
        um_set_pixel_power(true);
        esp_rom_delay_us(1000); // let the pixel power up before sending data
    }
}
#endif

void um_set_pixel_brightness(uint8_t b)
{
    um_ws_set_brightness(b);
    if (s_pixel_pwr)
        um_ws_refresh();
}
void um_set_pixel_color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
#if UM_GPIO_PIXEL >= 0
    pixel_power_on();
    um_ws_show_rgb(r, g, b);
#else
    UM_NOT_AVAILABLE();
#endif
}
void um_set_pixel_color_u32(uint32_t rgb) { um_set_pixel_color_rgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF); }
uint32_t um_color(uint8_t r, uint8_t g, uint8_t b) { return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b; }
uint32_t um_color_wheel(uint8_t pos)
{
    if (pos < 85)
        return um_color(255 - pos * 3, pos * 3, 0);
    if (pos < 170)
    {
        pos -= 85;
        return um_color(0, 255 - pos * 3, pos * 3);
    }
    pos -= 170;
    return um_color(pos * 3, 0, 255 - pos * 3);
}

// --- Blue LED ---
void um_set_blue_led(bool on)
{
#if UM_GPIO_BLUE_LED >= 0
    gpio_set_level(UM_GPIO_BLUE_LED, on ? 1 : 0);
    s_blue_led = on;
#else
    UM_NOT_AVAILABLE();
#endif
}
void um_toggle_blue_led(void) { um_set_blue_led(!s_blue_led); }

// --- ALS / VBUS ---
esp_err_t um_get_light_sensor_voltage(float *volts)
{
#if UM_GPIO_ALS >= 0
    if (!volts)
        return ESP_ERR_INVALID_ARG;
    if (!s_adc_als)
        return ESP_ERR_INVALID_STATE;
    int raw = 0;
    int mv = 0;
    ESP_RETURN_ON_ERROR(adc_oneshot_read(s_adc_als, s_adc_als_channel, &raw), TAG, "adc read");
    if (s_cali_als && adc_cali_raw_to_voltage(s_cali_als, raw, &mv) == ESP_OK)
    {
        *volts = mv / 1000.0f;
    }
    else
    {
        // rough fallback
        *volts = (float)raw / 4095.0f * 3.1f;
    }
    return ESP_OK;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

bool um_get_vbus_present(void)
{
#if UM_GPIO_VBUS_SENSE >= 0
    return !!gpio_get_level(UM_GPIO_VBUS_SENSE);
#else
    UM_NOT_AVAILABLE();
    return false;
#endif
}

// --- I2C bus ---
esp_err_t um_fg_setup(int i2c_port, int gpio_sda, int gpio_scl, uint32_t hz)
{
    ESP_RETURN_ON_ERROR(i2c_new_bus(i2c_port, gpio_sda, gpio_scl, hz), TAG, "i2c setup");
    return i2c_add_dev(UM_FG_ADDR, &s_fg_dev);
}

esp_err_t um_fg_setup_bus(i2c_master_bus_handle_t bus)
{
    if (!bus)
        return ESP_ERR_INVALID_ARG;
    if (s_bus && s_bus != bus)
        return ESP_ERR_INVALID_STATE; // on TinyC6[D], call this before um_begin()
    s_bus = bus;
    s_bus_owned = false;
    return i2c_add_dev(UM_FG_ADDR, &s_fg_dev);
}

i2c_master_bus_handle_t um_get_i2c_bus(void) { return s_bus; }

// --- Fuel Gauge (MAX17048) ---
static esp_err_t fg_read_u16(uint8_t reg, uint16_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;
    if (!s_fg_dev)
        return ESP_ERR_INVALID_STATE;
    uint8_t buf[2] = {0};
    ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(s_fg_dev, &reg, 1, buf, 2, UM_I2C_TIMEOUT_MS), TAG, "fg read");
    *out = ((uint16_t)buf[0] << 8) | buf[1];
    return ESP_OK;
}

esp_err_t um_fg_get_battery_voltage(float *volts)
{
    if (!volts)
        return ESP_ERR_INVALID_ARG;
    // Arduino helper: VCELL * 78.125uV
    uint16_t raw = 0;
    ESP_RETURN_ON_ERROR(fg_read_u16(0x02, &raw), TAG, "fg read vcell");
    *volts = (float)raw * 78.125f / 1000000.0f;
    return ESP_OK;
}

esp_err_t um_fg_get_version(uint8_t *ver)
{
    if (!ver)
        return ESP_ERR_INVALID_ARG;
    uint16_t raw = 0;
    ESP_RETURN_ON_ERROR(fg_read_u16(0x08, &raw), TAG, "fg read version");
    *ver = (uint8_t)(raw & 0xFF); // match Arduino helper behavior (low byte)
    return ESP_OK;
}

// --- IO expander (FXL6408) ---
#if defined(UM_HAS_IO_EXPANDER)
static esp_err_t iox_read(uint8_t reg, uint8_t *val)
{
    if (!s_iox_dev)
        return ESP_ERR_INVALID_STATE;
    return i2c_master_transmit_receive(s_iox_dev, &reg, 1, val, 1, UM_I2C_TIMEOUT_MS);
}

static esp_err_t iox_write(uint8_t reg, uint8_t val)
{
    if (!s_iox_dev)
        return ESP_ERR_INVALID_STATE;
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(s_iox_dev, buf, 2, UM_I2C_TIMEOUT_MS);
}

// Read-modify-write the bits in mask to set or clear
static esp_err_t iox_update(uint8_t reg, uint8_t mask, bool set)
{
    uint8_t data = 0;
    ESP_RETURN_ON_ERROR(iox_read(reg, &data), TAG, "iox read");
    return iox_write(reg, set ? (data | mask) : (data & ~mask));
}
#endif

esp_err_t um_iox_begin(void)
{
#if defined(UM_HAS_IO_EXPANDER)
    ESP_RETURN_ON_ERROR(i2c_new_bus(UM_I2C_PORT, UM_GPIO_I2C_SDA, UM_GPIO_I2C_SCL, UM_I2C_HZ), TAG, "i2c setup");
    ESP_RETURN_ON_ERROR(i2c_add_dev(UM_IOX_I2C_ADDR, &s_iox_dev), TAG, "iox add");
    // Manufacturer ID is 0b101 in bits 7:5. Reading also clears the reset interrupt flag
    uint8_t id = 0;
    ESP_RETURN_ON_ERROR(iox_read(FXL6408_DEVICE_ID, &id), TAG, "iox read id");
    return (id >> 5) == 0b101 ? ESP_OK : ESP_ERR_NOT_FOUND;
#else
    UM_NOT_AVAILABLE();
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t um_iox_pin_mode(uint8_t pin, um_iox_mode_t mode)
{
#if defined(UM_HAS_IO_EXPANDER)
    if (pin > 7)
        return ESP_ERR_INVALID_ARG;
    uint8_t mask = 1 << pin;
    if (mode == UM_IOX_OUTPUT)
    {
        ESP_RETURN_ON_ERROR(iox_update(FXL6408_IO_DIR, mask, true), TAG, "iox dir");
        // Outputs default to high-Z after reset, so enable the driver
        return iox_update(FXL6408_OUTPUT_HIGH_Z, mask, false);
    }
    ESP_RETURN_ON_ERROR(iox_update(FXL6408_IO_DIR, mask, false), TAG, "iox dir");
    ESP_RETURN_ON_ERROR(iox_update(FXL6408_PULL_UP_DOWN, mask, mode == UM_IOX_INPUT_PULLUP), TAG, "iox pull");
    return iox_update(FXL6408_PULL_ENABLE, mask, mode == UM_IOX_INPUT_PULLUP || mode == UM_IOX_INPUT_PULLDOWN);
#else
    UM_NOT_AVAILABLE();
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t um_iox_digital_write(uint8_t pin, bool level)
{
#if defined(UM_HAS_IO_EXPANDER)
    if (pin > 7)
        return ESP_ERR_INVALID_ARG;
    return iox_update(FXL6408_OUTPUT_STATE, 1 << pin, level);
#else
    UM_NOT_AVAILABLE();
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t um_iox_digital_read(uint8_t pin, bool *level)
{
#if defined(UM_HAS_IO_EXPANDER)
    if (pin > 7 || !level)
        return ESP_ERR_INVALID_ARG;
    // Reading the input status also clears any pending interrupt
    uint8_t data = 0;
    ESP_RETURN_ON_ERROR(iox_read(FXL6408_INPUT_STATUS, &data), TAG, "iox read");
    *level = (data >> pin) & 1;
    return ESP_OK;
#else
    UM_NOT_AVAILABLE();
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

// --- RF Switch ---
void um_set_antenna_external(bool external)
{
    // Set the RF Switch HIGH for External and LOW for Internal
#if defined(UM_HAS_IO_EXPANDER)
    if (um_iox_digital_write(UM_IOX_RF_SWITCH, external) != ESP_OK)
        ESP_LOGW(TAG, "RF switch write failed");
#else
    gpio_set_level(UM_GPIO_RF_SWITCH, external ? 1 : 0);
#endif
}

// --- Info ---
const char *um_board_name(void) { return UM_BOARD_NAME; }
