#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "ST7789";

// Pin definitions - adjust these according to your wiring
#define PIN_NUM_MOSI    19  // SDK
#define PIN_NUM_CLK     18  // SCL
#define PIN_NUM_CS      10  // CS
#define PIN_NUM_DC      8   // DC
#define PIN_NUM_RST     9   // RST
#define PIN_NUM_BL      7   // BL (Backlight)

// Display dimensions
#define LCD_WIDTH       240
#define LCD_HEIGHT      240

// ST7789 Commands
#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
#define ST7789_SLPOUT   0x11
#define ST7789_NORON    0x13
#define ST7789_INVOFF   0x20
#define ST7789_INVON    0x21
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_MADCTL   0x36
#define ST7789_COLMOD   0x3A

// Color definitions (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_WHITE     0xFFFF
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F

spi_device_handle_t spi;

// Send a command to the LCD
static void lcd_cmd(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    t.user = (void*)0; // D/C needs to be set to 0
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

// Send data to the LCD
static void lcd_data(const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0) return;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = data;
    t.user = (void*)1; // D/C needs to be set to 1
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

// This function is called before a transmission to set the D/C line
static void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

// Initialize the display
static void lcd_init(void)
{
    // Hardware reset
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Software reset
    lcd_cmd(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));

    // Sleep out
    lcd_cmd(ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Memory Data Access Control
    lcd_cmd(ST7789_MADCTL);
    uint8_t data = 0x00;
    lcd_data(&data, 1);

    // Interface Pixel Format - 16bit color (RGB565)
    lcd_cmd(ST7789_COLMOD);
    data = 0x55;
    lcd_data(&data, 1);

    // Normal display mode on
    lcd_cmd(ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Display on
    lcd_cmd(ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(10));
}

// Set the drawing window
static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // Column address set
    lcd_cmd(ST7789_CASET);
    uint8_t data[4];
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    lcd_data(data, 4);

    // Row address set
    lcd_cmd(ST7789_RASET);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    lcd_data(data, 4);

    // Write to RAM
    lcd_cmd(ST7789_RAMWR);
}

// Fill the screen with a color (RGB565 format)
static void lcd_fill_screen(uint16_t color)
{
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    
    // Allocate buffer for a line
    uint16_t *line_buf = heap_caps_malloc(LCD_WIDTH * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (line_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate line buffer");
        return;
    }

    // Fill line buffer with color (swap bytes for RGB565)
    uint16_t swapped_color = (color >> 8) | (color << 8);
    for (int i = 0; i < LCD_WIDTH; i++) {
        line_buf[i] = swapped_color;
    }

    // Send line by line
    for (int y = 0; y < LCD_HEIGHT; y++) {
        lcd_data((uint8_t*)line_buf, LCD_WIDTH * 2);
    }

    heap_caps_free(line_buf);
}

// Draw a filled rectangle
static void lcd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    lcd_set_window(x, y, x + w - 1, y + h - 1);

    uint16_t *line_buf = heap_caps_malloc(w * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (line_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        return;
    }

    uint16_t swapped_color = (color >> 8) | (color << 8);
    for (int i = 0; i < w; i++) {
        line_buf[i] = swapped_color;
    }

    for (int i = 0; i < h; i++) {
        lcd_data((uint8_t*)line_buf, w * 2);
    }

    heap_caps_free(line_buf);
}

void app_main(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Initializing ST7789 display");

    // Initialize non-SPI GPIOs
    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_RST) | (1ULL << PIN_NUM_BL)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // Turn on backlight
    gpio_set_level(PIN_NUM_BL, 1);

    // Initialize SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 2 + 8
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // Attach the LCD to the SPI bus
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000,  // 40 MHz
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = lcd_spi_pre_transfer_callback,
    };

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    // Initialize the LCD
    lcd_init();
    
    ESP_LOGI(TAG, "Display initialized successfully");

    // Demo: Fill screen with different colors and draw shapes
    while (1) {
        ESP_LOGI(TAG, "Filling screen with RED");
        lcd_fill_screen(COLOR_RED);
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(TAG, "Filling screen with GREEN");
        lcd_fill_screen(COLOR_GREEN);
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(TAG, "Filling screen with BLUE");
        lcd_fill_screen(COLOR_BLUE);
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(TAG, "Drawing rectangles");
        lcd_fill_screen(COLOR_BLACK);
        lcd_draw_rect(20, 20, 80, 60, COLOR_RED);
        lcd_draw_rect(140, 20, 80, 60, COLOR_GREEN);
        lcd_draw_rect(20, 160, 80, 60, COLOR_BLUE);
        lcd_draw_rect(140, 160, 80, 60, COLOR_YELLOW);
        vTaskDelay(pdMS_TO_TICKS(3000));

        ESP_LOGI(TAG, "Drawing stripes");
        lcd_fill_screen(COLOR_BLACK);
        for (int i = 0; i < LCD_WIDTH; i += 30) {
            lcd_draw_rect(i, 0, 15, LCD_HEIGHT, COLOR_CYAN);
        }
        vTaskDelay(pdMS_TO_TICKS(3000));

        ESP_LOGI(TAG, "Drawing gradient");
        lcd_fill_screen(COLOR_BLACK);
        for (int i = 0; i < LCD_HEIGHT; i += 10) {
            uint16_t color = (i * 0xFFFF) / LCD_HEIGHT;
            lcd_draw_rect(0, i, LCD_WIDTH, 10, color);
        }
        vTaskDelay(pdMS_TO_TICKS(3000));

        ESP_LOGI(TAG, "Filling screen with WHITE");
        lcd_fill_screen(COLOR_WHITE);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}