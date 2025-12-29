#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "st7789.h"
#include "images.h"

static const char *TAG = "ST7789";



// Color definitions (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_WHITE     0xFFFF
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F


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
        .pre_cb = lcd_spi_pre_transfer_callback,   // we are setting ESP-IDF to call this function before every SPI transmission on the device
    };

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    // Initialize the LCD
    lcd_init();
    
    ESP_LOGI(TAG, "Display initialized successfully");

    // Demo: Fill screen with different colors and draw shapes
    while (1) {

      // Draw full-screen image (240x240 example)
        lcd_fill_screen(COLOR_BLACK);
        lcd_draw_image(0, 0,240, 240,(const uint16_t *)om_img );
        vTaskDelay(pdMS_TO_TICKS(10000));
        // Example 1: Simple text
        int i = 10;
        char buf[20];
        snprintf(buf, 20, "Outdoor Temp = %d ", i);
        snprintf(buf, 20, buf,"℃");
        ESP_LOGI(TAG, "Drawing text demo");
        lcd_fill_screen(COLOR_BLACK);
        lcd_draw_string(10, 10, buf, COLOR_WHITE, COLOR_BLACK,1);
        lcd_draw_string(10, 19, "ESP32 Display", COLOR_CYAN, COLOR_BLACK,2);
        lcd_draw_string(10, 34, "Text at X,Y", COLOR_YELLOW, COLOR_BLACK,3);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Example 2: Different colors
        lcd_fill_screen(COLOR_BLUE);
        lcd_draw_string(20, 20, "RED TEXT", COLOR_RED, COLOR_BLUE,1);
        lcd_draw_string(20, 40, "GREEN TEXT", COLOR_GREEN, COLOR_BLUE,2);
        lcd_draw_string(20, 60, "WHITE TEXT", COLOR_WHITE, COLOR_BLUE,3);
        lcd_draw_string(20, 80, "YELLOW TEXT", COLOR_YELLOW, COLOR_BLUE,4);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Example 3: Multiple lines
        lcd_fill_screen(COLOR_BLACK);
        lcd_draw_string(5, 10, "Line 1: Hello!", COLOR_WHITE, COLOR_BLACK,1);
        lcd_draw_string(5, 25, "Line 2: ESP32", COLOR_GREEN, COLOR_BLACK,2);
        lcd_draw_string(5, 40, "Line 3: ST7789", COLOR_CYAN, COLOR_BLACK,3);
        lcd_draw_string(5, 55, "Line 4: Display", COLOR_YELLOW, COLOR_BLACK,1);
        lcd_draw_string(5, 70, "Line 5: Driver", COLOR_MAGENTA, COLOR_BLACK,2);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Example 4: Centered text
        lcd_fill_screen(COLOR_WHITE);
        const char *msg = "CENTERED";
        int text_width = strlen(msg) * (FONT_WIDTH + FONT_SPACING);
        int center_x = (LCD_WIDTH - text_width) / 2;
        int center_y = (LCD_HEIGHT - FONT_HEIGHT) / 2;
        lcd_draw_string(center_x, center_y, msg, COLOR_RED, COLOR_WHITE,2);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Example 5: Numbers and symbols
        lcd_fill_screen(COLOR_BLACK);
        lcd_draw_string(10, 20, "0123456789", COLOR_WHITE, COLOR_BLACK,1);
        lcd_draw_string(10, 40, "!@#$%^&*()", COLOR_CYAN, COLOR_BLACK,2);
        lcd_draw_string(10, 60, "ABCDEFGHIJ", COLOR_GREEN, COLOR_BLACK,3);
        lcd_draw_string(10, 80, "abcdefghij", COLOR_YELLOW, COLOR_BLACK,1);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Example 6: Text with shapes
        lcd_fill_screen(COLOR_BLACK);
        lcd_draw_rect(5, 5, 230, 30, COLOR_BLUE);
        lcd_draw_string(15, 12, "TEXT IN BOX", COLOR_WHITE, COLOR_BLUE,1);
        
        lcd_draw_rect(5, 45, 230, 30, COLOR_RED);
        lcd_draw_string(15, 52, "ANOTHER BOX", COLOR_WHITE, COLOR_RED,2);
        
        lcd_draw_rect(5, 85, 230, 30, COLOR_GREEN);
        lcd_draw_string(15, 92, "THIRD BOX!", COLOR_BLACK, COLOR_GREEN,3);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}