#include "st7789.h"
#include "esp_log.h"
static const char *TAG = "ST7789";
spi_device_handle_t spi;        // handle from spi_master.h esp driver
/*
typedef struct spi_device_t *spi_device_handle_t;  ///< Handle for a device on a SPI bus
struct spi_device_t {

    int id;
    QueueHandle_t trans_queue;
    QueueHandle_t ret_queue;
    spi_device_interface_config_t cfg;
    spi_hal_dev_config_t hal_dev;
    spi_host_t *host;
    spi_bus_lock_dev_handle_t dev_lock;

};


typedef struct spi_transaction_t spi_transaction_t;
spi_transaction_t = [flags, cmd, addr, length, rxlength, *user, union{*tx_buffer,tx_data}, union{*rx_buffer, rx_data}]
*/

// Send a command to the LCD
void lcd_cmd(const uint8_t cmd)
{
    esp_err_t ret;              //Will store the return status (success/error)
    spi_transaction_t t;        //SPI transaction structure that holds all transmission info
    memset(&t, 0, sizeof(t));   //Sets all bytes in structure t to zero
    t.length = 8;               //Number of bits to transmit  8=1byte  if sending 2 bytes use t.length = 16
    t.tx_buffer = &cmd;         // "tx_buffer" Pointer to the data to send &cmd = address of command byte 
    t.user = (void*)0;          // D/C needs to be set to 0 for command  [This value is used in the callback function to set the DC pin]
    ret = spi_device_polling_transmit(spi, &t);     //Sends data and waits until complete (blocking)
                                                    //spi = The SPI device handle (our display)
                                                    //&t = Pointer to transaction structure
                                                    //ret = Stores the result (ESP_OK if successful)
    assert(ret == ESP_OK);      //If condition is false, program crashes with error message
}

// Send data to the LCD
void lcd_data(const uint8_t *data, int len)
{
    esp_err_t ret;          //store return status
    spi_transaction_t t;    //SPI transaction structure
    if (len == 0) return;   
    memset(&t, 0, sizeof(t));   //set all bytes to zero(0)
    t.length = len * 8;         // 8 bits to transmit
    t.tx_buffer = data;         //data to transmit
    t.user = (void*)1; // D/C needs to be set to 1  [within the structure (spi_transaction_t t) we are using "user" bit for DC]
    ret = spi_device_polling_transmit(spi, &t);     //send the data
    assert(ret == ESP_OK); //error check
}

/*
DC pin simply means Data/Command
DC = 0 (LOW) → Command
DC = 1 (High)→ Data

You can also observe lcd_cmd function t.user = 0 (Command)
and lcd_data function t.user = 1 (Data)
*/

// This function is called before a transmission to set the D/C line
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;  //within the structure (spi_transaction_t t) we are using "user" bit for DC pin
    gpio_set_level(PIN_NUM_DC, dc);
}


// Draw a single character at x, y position
void lcd_draw_char(
    uint16_t x,
    uint16_t y,
    char c,
    uint16_t color,
    uint16_t bg_color,
    uint8_t scale
)
{
    if (c < 32 || c > 126) c = '?';

    uint16_t char_w = FONT_WIDTH * scale;
    uint16_t char_h = FONT_HEIGHT * scale;

    if (x + char_w > LCD_WIDTH || y + char_h > LCD_HEIGHT) return;

    const uint8_t *char_bitmap = font_5x7[c - 32];

    lcd_set_window(x, y, x + char_w - 1, y + char_h - 1);

    uint16_t *buf = heap_caps_malloc(char_w * char_h * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!buf) {
        ESP_LOGE(TAG, "Char buffer alloc failed");
        return;
    }

    uint16_t fg = (color >> 8) | (color << 8);
    uint16_t bg = (bg_color >> 8) | (bg_color << 8);

    int idx = 0;

    for (int row = 0; row < FONT_HEIGHT; row++) {
        for (int sy = 0; sy < scale; sy++) {
            for (int col = 0; col < FONT_WIDTH; col++) {
                uint16_t pixel = (char_bitmap[col] & (1 << row)) ? fg : bg;
                for (int sx = 0; sx < scale; sx++) {
                    buf[idx++] = pixel;
                }
            }
        }
    }

    lcd_data((uint8_t *)buf, char_w * char_h * 2);
    heap_caps_free(buf);
}

// Initialize the display
void lcd_init(void)
{
    // Hardware reset
    gpio_set_level(PIN_NUM_RST, 0);  // 0 = Low    Pull RST LOW  → Hold display in reset
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_NUM_RST, 1);  // 1 = HIGH   Pull RST HIGH → Release from reset, display starts up
    vTaskDelay(pdMS_TO_TICKS(100));

    // Software reset
    lcd_cmd(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));

    // Sleep out
    lcd_cmd(ST7789_SLPOUT); // Turn off the sleep mode
    vTaskDelay(pdMS_TO_TICKS(120));

    // Memory Data Access Control
    lcd_cmd(ST7789_MADCTL); //Send COMMAND byte
    uint8_t data = 0x00;
    lcd_data(&data, 1); //Send DATA byte(s) for that command

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
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
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
void lcd_fill_screen(uint16_t color)
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

//Gradient Fill
void lcd_fill_gradient(uint16_t start_color, uint16_t end_color)
{
    // 1. Set window to full screen
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    // 2. Allocate buffer for ONE line (same as before)
    uint16_t *line_buf = heap_caps_malloc(LCD_WIDTH * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (line_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate line buffer");
        return;
    }

    // 3. Loop through every vertical line (y)
    for (int y = 0; y < LCD_HEIGHT; y++) {
        
        // Calculate the "ratio" for this specific line (0 to 255)
        // At y=0, ratio is 0. At y=Height, ratio is 255.
        uint8_t ratio = (y * 255) / LCD_HEIGHT;

        // Get the color for THIS row
        uint16_t current_color = lerp_rgb565(start_color, end_color, ratio);

        // Swap bytes (Little Endian -> Big Endian for LCD)
        uint16_t swapped_color = (current_color >> 8) | (current_color << 8);

        // Fill the line buffer with this new color
        for (int x = 0; x < LCD_WIDTH; x++) {
            line_buf[x] = swapped_color;
        }

        // Send this line to the display
        lcd_data((uint8_t*)line_buf, LCD_WIDTH * 2);
    }

    // 4. Cleanup
    heap_caps_free(line_buf);
}

// Helper: Linear Interpolation for RGB565 colors
// ratio: 0 to 255 (0 = start_color, 255 = end_color)
uint16_t lerp_rgb565(uint16_t start_color, uint16_t end_color, uint8_t ratio)
{
    // 1. Unpack Start Color (RGB565 -> R, G, B components)
    uint16_t r1 = (start_color >> 11) & 0x1F; // 5 bits Red
    uint16_t g1 = (start_color >> 5)  & 0x3F; // 6 bits Green
    uint16_t b1 =  start_color        & 0x1F; // 5 bits Blue

    // 2. Unpack End Color
    uint16_t r2 = (end_color >> 11) & 0x1F;
    uint16_t g2 = (end_color >> 5)  & 0x3F;
    uint16_t b2 =  end_color        & 0x1F;

    // 3. Interpolate (Mix) the components based on ratio
    // Formula: result = start + (end - start) * ratio / 255
    uint16_t r = r1 + ((int32_t)(r2 - r1) * ratio / 255);
    uint16_t g = g1 + ((int32_t)(g2 - g1) * ratio / 255);
    uint16_t b = b1 + ((int32_t)(b2 - b1) * ratio / 255);

    // 4. Repack into RGB565
    return (r << 11) | (g << 5) | b;
}

// Draw a filled rectangle
void lcd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    // It checks if the starting x or y coordinates are completely outside the visible screen area. 
    //If so, it returns immediately to avoid errors.
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x; // Clipping the screen to fit
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y; // Clipping the screen to fit

    lcd_set_window(x, y, x + w - 1, y + h - 1); //defining the "active area" or "address window."
    /*The -1 is used because screen coordinates are usually 0-indexed
    This tells the LCD: "I am about to send you pixels. Start at (x,y). Fill the width w.
    Once you fill a row, automatically wrap around to the next row until you hit height h."
    */

    uint16_t *line_buf = heap_caps_malloc(w * sizeof(uint16_t), MALLOC_CAP_DMA);
    /*Instead of sending pixels one by one (slow) or allocating a huge buffer for the whole rectangle (memory intensive),
     it allocates just enough memory for one horizontal row of the rectangle.
     */
    if (line_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        return;
    }

    uint16_t swapped_color = (color >> 8) | (color << 8);
    for (int i = 0; i < w; i++) {
        line_buf[i] = swapped_color;
        //line_buf Memory: [ RED ] [ RED ] [ RED ] [ RED ] ... [ RED ]
    }

    for (int i = 0; i < h; i++) {  // iternate h time "Height of the rectangle"
        lcd_data((uint8_t*)line_buf, w * 2); // 1 Pixel = 2 Bytes hence we are doing w*2
    }

    heap_caps_free(line_buf);
}

// Draw a filled rectangle
void lcd_draw_rect_empty(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t thickness, uint16_t color)
{
    //(x,y) are the Top left corner of the rectangle that we want
    // W= Horizontal Wdith of the rectangele
    // h= Vertical Height of the rectangle
    lcd_draw_rect(x,y,w,thickness,color); // Top Horizontal Line
    lcd_draw_rect(x,y+h-thickness,w,thickness,color); // bottom Horizontal Line
    lcd_draw_rect(x,y,thickness,h,color); // Left Vertical Line
    lcd_draw_rect(x+w-thickness,y,thickness,h,color); // Right Vertical Line
}


// Draw a string at x, y position
void lcd_draw_string(
    uint16_t x,
    uint16_t y,
    const char *str,
    uint16_t color,
    uint16_t bg_color,
    uint8_t scale
)
{
    uint16_t cx = x;

    while (*str) {
        if (cx + FONT_WIDTH * scale > LCD_WIDTH) {
            cx = x;
            y += (FONT_HEIGHT + 2) * scale;
            if (y + FONT_HEIGHT * scale > LCD_HEIGHT) break;
        }

        lcd_draw_char(cx, y, *str, color, bg_color, scale);
        cx += (FONT_WIDTH + FONT_SPACING) * scale;
        str++;
    }
}

void lcd_draw_image(uint16_t x, uint16_t y,
                    uint16_t w, uint16_t h,
                    const uint16_t *img)
{
    lcd_set_window(x, y, x + w - 1, y + h - 1);

    for (int row = 0; row < h; row++) {
        lcd_data((uint8_t *)&img[row * w], w * 2);
    }
}

void lcd_draw_image_invert(uint16_t x, uint16_t y,
                    uint16_t w, uint16_t h,
                    const uint16_t *img)
{
    lcd_set_window(x, y, x + w - 1, y + h - 1);

    /*
    The color inversion is happening because your image data is already in RGB565 format, 
    but you're not doing the byte swapping that the ST7789 expects. 
    The ST7789 needs RGB565 data with swapped byte order (big-endian), but your image is likely in little-endian format.
    */

    // Allocate buffer for byte-swapped data
    uint16_t *line_buf = heap_caps_malloc(w * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (line_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate image buffer");
        return;
    }

    for (int row = 0; row < h; row++) {
        // Swap bytes for each pixel in the row
        for (int col = 0; col < w; col++) {
            uint16_t pixel = img[row * w + col];
            line_buf[col] = (pixel >> 8) | (pixel << 8);  // Swap bytes
        }
        
        // Send the swapped row
        lcd_data((uint8_t *)line_buf, w * 2);
    }

    heap_caps_free(line_buf);
}

