#include "images.h"
#include <stdint.h>

// Correct linker-generated symbols
extern const uint8_t _binary_om_bin_start[];
extern const uint8_t _binary_om_bin_end[];

// Pointer to RGB565 image data
const uint16_t *om_img =
    (const uint16_t *)_binary_om_bin_start;

// Runtime-safe size function
uint32_t om_size(void)
{
    return (uint32_t)(
        _binary_om_bin_end -
        _binary_om_bin_start
    );
}
