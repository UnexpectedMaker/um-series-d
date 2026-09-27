#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "esp_check.h"
#include "esp_log.h"
#include "soc/soc_caps.h"
#include "sdkconfig.h"
#include "um_seriesd_pins.h"

static const char *TAG = "um_ws2812";

// 800kHz WS2812 @ 10MHz RMT resolution (100ns ticks) - same timing as the Arduino helper
#define WS_RES_HZ (10 * 1000 * 1000)
#define WS_T0H_NS 400
#define WS_T0L_NS 800
#define WS_T1H_NS 800
#define WS_T1L_NS 400
#define WS_RESET_NS 350000 // >280us latch for newer WS2812B
#define WS_TX_TIMEOUT_MS 100

typedef struct
{
    rmt_channel_handle_t chan;
    rmt_encoder_handle_t encoder;
    int gpio;
    uint8_t brightness; // 0..255
    uint8_t rgb[3];     // last color set, re-sent when the brightness changes
} um_ws_t;

static um_ws_t s_ws = {.chan = NULL, .encoder = NULL, .gpio = UM_GPIO_PIXEL, .brightness = CONFIG_UM_PIXEL_DEFAULT_BRIGHTNESS};

static inline uint8_t scale8(uint8_t v, uint8_t b)
{
    return ((uint16_t)v * ((uint16_t)b + 1)) >> 8;
}

static rmt_symbol_word_t ns_to_symbol(uint32_t th, uint32_t tl)
{
    // 100ns tick
    return (rmt_symbol_word_t){.level0 = 1, .duration0 = th / 100, .level1 = 0, .duration1 = tl / 100};
}

esp_err_t um_ws_init(void)
{
    if (s_ws.gpio < 0 || s_ws.chan)
        return ESP_OK;
    rmt_tx_channel_config_t cfg = {
        .gpio_num = s_ws.gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = WS_RES_HZ,
        .mem_block_symbols = SOC_RMT_MEM_WORDS_PER_CHANNEL, // one block, fits 24 bits + reset
        .trans_queue_depth = 4};
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&cfg, &s_ws.chan), TAG, "new tx");
    rmt_copy_encoder_config_t enc_cfg = {};
    ESP_RETURN_ON_ERROR(rmt_new_copy_encoder(&enc_cfg, &s_ws.encoder), TAG, "new encoder");
    ESP_RETURN_ON_ERROR(rmt_enable(s_ws.chan), TAG, "enable");
    return ESP_OK;
}

void um_ws_deinit(void)
{
    if (s_ws.chan)
    {
        rmt_disable(s_ws.chan);
        rmt_del_channel(s_ws.chan);
        s_ws.chan = NULL;
    }
    if (s_ws.encoder)
    {
        rmt_del_encoder(s_ws.encoder);
        s_ws.encoder = NULL;
    }
}

void um_ws_set_brightness(uint8_t b) { s_ws.brightness = b; }

esp_err_t um_ws_refresh(void)
{
    if (!s_ws.chan)
        return ESP_OK;
    uint8_t grb[3] = {scale8(s_ws.rgb[1], s_ws.brightness), scale8(s_ws.rgb[0], s_ws.brightness), scale8(s_ws.rgb[2], s_ws.brightness)};

    const rmt_symbol_word_t sym0 = ns_to_symbol(WS_T0H_NS, WS_T0L_NS);
    const rmt_symbol_word_t sym1 = ns_to_symbol(WS_T1H_NS, WS_T1L_NS);
    rmt_symbol_word_t items[3 * 8 + 1];

    size_t idx = 0;
    for (int i = 0; i < 3; ++i)
    {
        for (int bit = 7; bit >= 0; --bit)
        {
            items[idx++] = (grb[i] & (1 << bit)) ? sym1 : sym0;
        }
    }
    // Hold the line low for the reset/latch time
    items[idx++] = (rmt_symbol_word_t){.level0 = 0, .duration0 = WS_RESET_NS / 200, .level1 = 0, .duration1 = WS_RESET_NS / 200};

    rmt_transmit_config_t tx = {.loop_count = 0};
    ESP_RETURN_ON_ERROR(rmt_transmit(s_ws.chan, s_ws.encoder, items, sizeof(items), &tx), TAG, "transmit");
    // items is on the stack, so wait for the transmit to finish before returning
    return rmt_tx_wait_all_done(s_ws.chan, WS_TX_TIMEOUT_MS);
}

esp_err_t um_ws_show_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    s_ws.rgb[0] = r;
    s_ws.rgb[1] = g;
    s_ws.rgb[2] = b;
    return um_ws_refresh();
}
