#ifndef ST7789_SCREEN_WRITE_HELPER
#define ST7789_SCREEN_WRITE_HELPER

#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include <stdbool.h>

void st7789_wakeup(spi_device_handle_t dev_handle);
void draw_characters(spi_device_handle_t dev_handle, const char* user_text, int x, int y);
void clear_screen(spi_device_handle_t dev_handle);
spi_device_handle_t st7789_screen_create_handle();

#endif