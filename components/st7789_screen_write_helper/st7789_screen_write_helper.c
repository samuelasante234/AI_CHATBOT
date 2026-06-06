#include "st7789_screen_write_helper.h"
#include "st7789_screen_driver.h"
#include "esp_rom_sys.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_cache.h"
#include <stdbool.h>
#include "driver/gpio.h"
#include "soc/gpio_reg.h"
#include "esp_heap_caps.h"
#include <stdint.h>
#include <string.h>
#include "font_module.h"

#define MAX_NO_OF_CHARACTERS 30

static void send_pixels(spi_device_handle_t dev_handle, uint16_t *colour, uint32_t len);
static void st7789_set_window(spi_device_handle_t dev_handle, uint16_t xs, uint16_t xe, uint16_t ys, uint16_t ye);
void st7789_wakeup(spi_device_handle_t dev_handle);
static void st7789_fill_area(spi_device_handle_t dev_handle, uint16_t x, uint16_t y, bool is_whole_screen);
void draw_characters(spi_device_handle_t dev_handle, const char* user_text, int x, int y);
void clear_screen(spi_device_handle_t dev_handle);
static uint16_t* convert_text_pixels(uint8_t *text, int no_of_characters);
static int get_index(int i, int num_of_chars);
spi_device_handle_t st7789_screen_create_handle();

spi_device_handle_t st7789_screen_create_handle() {
    return st7789_init();
}
static void send_pixels(spi_device_handle_t dev_handle, uint16_t* colour, uint32_t len) {
    spi_transaction_t transact_t = {0};
    static uint16_t *head =NULL;
    if (!head) {head = (uint16_t *)heap_caps_aligned_calloc(64,1,(uint32_t)MAX_TRANSFER_SIZE*2*2,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);}
    if (!head) {printf("Couldn't allocate dma buffer"); fflush(stdout); return;}
    for (uint32_t i=0; i<len;i++) {
        *(head +i) = *(colour+i)<<8 | *(colour+i) >> 8;
    }
    esp_err_t result = esp_cache_msync((void*)head,len*2,ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_TYPE_DATA);
    if (result != ESP_OK) {
        printf("Couldn't sync! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    uint16_t max_pixels = 16000; uint32_t pixels_sent=0;
    while (pixels_sent<len) {
        if (len-pixels_sent>max_pixels) {
            transact_t.tx_buffer= head +pixels_sent; transact_t.length=max_pixels*8*2; transact_t.user = (void*)1;
            transact_t.rxlength=0;
            result = spi_device_transmit(dev_handle,&transact_t);
            if (result != ESP_OK) {
                printf("Couldn't transmit pixels! Error: %s\n", esp_err_to_name(result));
                fflush(stdout);
                return;
            }
            pixels_sent+=(max_pixels);
        }
        else {
            transact_t.tx_buffer=head+pixels_sent; transact_t.length=(len-pixels_sent)*8*2; transact_t.user=(void*)1;
            transact_t.rxlength=0;
            result = spi_device_transmit(dev_handle,&transact_t);
            if (result != ESP_OK) {
                printf("Couldn't transmit pixels! Error: %s\n", esp_err_to_name(result));
                fflush(stdout);
                return;
            }
            pixels_sent+=(len-pixels_sent);
        }
    }
    
}
void st7789_wakeup(spi_device_handle_t dev_handle) {
    gpio_set_direction(RES_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(DC_PIN, GPIO_MODE_OUTPUT);
    volatile uint32_t *Overhead_set = (volatile uint32_t *) GPIO_OUT_W1TS_REG;
    volatile uint32_t *Overhead_clear = (volatile uint32_t *) GPIO_OUT_W1TC_REG;
    *Overhead_set = 1<<RES_PIN;
    vTaskDelay(10);
    *Overhead_clear = 1<<RES_PIN;
    vTaskDelay(50);
    //esp_rom_delay_us(14);
    *Overhead_set=1<<RES_PIN;
    vTaskDelay(120);
    send_command(dev_handle, 0x01); //software reset
    vTaskDelay(120);
    send_command(dev_handle, 0x11); //sleep out
    vTaskDelay(120);
    send_command(dev_handle, 0x3A); //color mode
    uint8_t temp = 0x55;
    send_data(dev_handle,&temp, 1);
    send_command(dev_handle, 0x36); //memory address control
    temp = 0x00;
    send_data(dev_handle,&temp, 1);
    send_command(dev_handle,0x21); //display inversion
    send_command(dev_handle,0x29); //display on
}
static void st7789_set_window(spi_device_handle_t dev_handle, uint16_t xs, uint16_t xe, uint16_t ys, uint16_t ye) {
    send_command(dev_handle, 0x2A); //CASET
    uint8_t temp[4]={xs>>8, xs, xe>>8, xe};
    send_data(dev_handle,temp,4);
    send_command(dev_handle, 0x2B); //RASET
    temp[0]=ys>>8, temp[1]=ys, temp[2]=ye>>8, temp[3]=ye;
    send_data(dev_handle,temp,4);
    send_command(dev_handle, 0x2C); //RAMWR
}
static void st7789_fill_area(spi_device_handle_t dev_handle, uint16_t x, uint16_t y,bool is_whole_screen) {
    uint16_t width=SCREEN_WIDTH, height;
    if (is_whole_screen) height=SCREEN_HEIGHT;
    else height=FONT_HEIGHT;
    if ((y+height) >SCREEN_HEIGHT && !(is_whole_screen)) {
        height=SCREEN_HEIGHT, height -=y;
    }
    st7789_set_window(dev_handle, x,x + width-1, y, y + height-1);
}

static uint16_t* convert_text_pixels(uint8_t *text, int no_of_characters) {
    static uint16_t *Overhead = NULL;
    if(!Overhead) Overhead = (uint16_t*)heap_caps_aligned_calloc(64, 1,(uint32_t)(SCREEN_HEIGHT*SCREEN_WIDTH*2),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if (!Overhead) {
        return NULL;
    }
    int k=0;
    for (int i=0; i<FONT_HEIGHT*no_of_characters; i++) {
        for (int j=FONT_WIDTH-1; j>=0;j--, k++){
            if (*(text+get_index(i, no_of_characters))&(1<<j)) *(Overhead+k) = 0x07FF;
            else *(Overhead+k) =0x18E3;
        }
    }
    esp_err_t result = esp_cache_msync((void*) Overhead,(uint32_t)(no_of_characters)*FONT_HEIGHT*FONT_WIDTH*2,ESP_CACHE_MSYNC_FLAG_DIR_C2M|ESP_CACHE_MSYNC_FLAG_TYPE_DATA);
    if (result != ESP_OK) {
        printf("Couldn't sync memory! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    return Overhead;
}
static int get_index(int i, int num_of_chars) {
    int row = i/num_of_chars, column=i%num_of_chars;
    return FONT_HEIGHT*column + row;
}
void draw_characters(spi_device_handle_t dev_handle, const char* user_text, int x, int y) {
    int no_of_characters = strlen(user_text),l;
    uint8_t converted_text[FONT_HEIGHT*MAX_NO_OF_CHARACTERS];
    char padded_buffer[MAX_NO_OF_CHARACTERS+1];
    padded_buffer[MAX_NO_OF_CHARACTERS] ='\0';
    int left_right_x=x-0;
    for (l=0;(left_right_x-FONT_WIDTH)>=0;l++) left_right_x-=FONT_WIDTH,padded_buffer[l]=' ';
    if (left_right_x) padded_buffer[l]=' ',l++, x+=(FONT_WIDTH-left_right_x);
    for (int k=0;k<no_of_characters && (l<MAX_NO_OF_CHARACTERS);l++,k++) padded_buffer[l]=*(user_text+k);
    left_right_x=SCREEN_WIDTH-(x +(FONT_WIDTH*no_of_characters));
    for (;(left_right_x-FONT_WIDTH)>=0;l++) left_right_x-=FONT_WIDTH,padded_buffer[l]=' ';
    for (int i=0; i<MAX_NO_OF_CHARACTERS;i++) {
        for (int k=0; k<(FONT_HEIGHT); k++) {
            converted_text[16*i+k]= ascii_8x16_font[*(padded_buffer+i)-32][k];
        }
    }
    uint16_t *ptr_to_text= convert_text_pixels(converted_text,MAX_NO_OF_CHARACTERS);
    if (!ptr_to_text) {
        printf("Couldn't reserve space!");
        fflush(stdout);
        return;
    }
    st7789_fill_area(dev_handle,0,y,false);
    send_pixels(dev_handle,ptr_to_text,FONT_HEIGHT*FONT_WIDTH*(uint32_t)MAX_NO_OF_CHARACTERS);
}
void clear_screen(spi_device_handle_t dev_handle) {
    static uint16_t *Overheader = NULL;
    if(!Overheader) Overheader = (uint16_t*)heap_caps_aligned_calloc(64, 1,(uint32_t)(SCREEN_HEIGHT*SCREEN_WIDTH*2),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if (!Overheader) {
        return;
    }
    for (int i=0; i<(SCREEN_HEIGHT*SCREEN_WIDTH);i++) {
        *(Overheader +i)=0x18E3;
    }
    esp_err_t result = esp_cache_msync((void*) Overheader,SCREEN_HEIGHT*SCREEN_WIDTH*2,ESP_CACHE_MSYNC_FLAG_DIR_C2M|ESP_CACHE_MSYNC_FLAG_TYPE_DATA);
    if (result != ESP_OK) {
        printf("Couldn't sync memory! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    st7789_fill_area(dev_handle,0,0,true);
    send_pixels(dev_handle,Overheader,SCREEN_HEIGHT*SCREEN_WIDTH);
}