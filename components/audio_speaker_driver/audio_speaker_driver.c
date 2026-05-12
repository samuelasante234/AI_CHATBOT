#include "audio_speaker_driver.h"
#include "driver/i2s_common.h"
#include "driver/i2s_std.h"
#include <stdbool.h>
#include <stdio.h>
#include "esp_err.h"
#include <stdint.h>
#include <math.h>
#include "freertos/FreeRTOS.h"

#define SOC_MOD_CLK_PLL_F80M 5

i2s_chan_handle_t amplifier_init();
void amplifier_amplify(i2s_chan_handle_t amplifier_handle);
static const int32_t zero_initialise[512]={0};
static int32_t sinewave_values[512];

i2s_chan_handle_t amplifier_init() {
    i2s_chan_config_t channel_config ={
        .dma_desc_num=6,
        .dma_frame_num=512,
        .id=I2S_NUM_1,
        .intr_priority=0,
        .role=I2S_ROLE_MASTER,
        .allow_pd=false,
        .auto_clear_before_cb=true,
    };
    i2s_chan_handle_t amplifier_handle;
    esp_err_t result;
    result=i2s_new_channel(&channel_config,&amplifier_handle,NULL);
    if (result != ESP_OK) {
        printf("Couldn't configure channel for amplifier I2S! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    i2s_std_slot_config_t amplifier_slot_config={
        .ws_width=0,
        .ws_pol=false,
        .slot_mode=I2S_SLOT_MODE_MONO,
        .slot_mask=I2S_STD_SLOT_LEFT,
        .slot_bit_width=I2S_SLOT_BIT_WIDTH_32BIT,
        .left_align=true,
        .data_bit_width=I2S_DATA_BIT_WIDTH_32BIT,
        .bit_shift=true,
        .bit_order_lsb=false,
        .big_endian=false,
    };
    i2s_std_clk_config_t amplifier_clock_config={
        .sample_rate_hz=16000,
        .clk_src=I2S_CLK_SRC_DEFAULT,
        .mclk_multiple=64,
    };
    i2s_std_gpio_config_t amplifier_gpio_config={
        .bclk=GPIO_NUM_6,
        .din=GPIO_NUM_NC,
        .dout=GPIO_NUM_5,
        .mclk=GPIO_NUM_NC,
        .ws=GPIO_NUM_7,
    };
    i2s_std_config_t amplifer_all_config={
        .clk_cfg=amplifier_clock_config,
        .gpio_cfg=amplifier_gpio_config,
        .slot_cfg=amplifier_slot_config,
    };
    result=i2s_channel_init_std_mode(amplifier_handle,&amplifer_all_config);
    if (result != ESP_OK) {
        printf("Couldn't configure I2S peripherals! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    size_t bytes;
    result=i2s_channel_preload_data(amplifier_handle,(const void*)zero_initialise,sizeof(zero_initialise),&bytes);
    if (result != ESP_OK) {
        printf("Couldn't zero out DMA buffer! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    for (uint16_t i=0,j=0; j<512;j++,i++, i%=512) {
        sinewave_values[j] = (double)(2147483647*0.05)*sin(2*3.14*1000*(double)(i)/(double)(16000));
    }
    return amplifier_handle;
}
void amplifier_amplify(i2s_chan_handle_t amplifier_handle) {
    if (!amplifier_handle) {
        printf("Amplifier handle does not exist!\n");
        fflush(stdout);
        return;
    }
    esp_err_t result = i2s_channel_enable(amplifier_handle);
    if (result != ESP_OK) {
        printf("Couldn't spit out DMA buffer onto amplifier! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    while (1) {
        result = i2s_channel_write(amplifier_handle,(const void*)sinewave_values,sizeof(sinewave_values),NULL,portMAX_DELAY);
        if (result != ESP_OK) {
            printf("Couldn't write to DMA buffer! Error: %s\n",esp_err_to_name(result));
            fflush(stdout);
            return;
    }
    }   
}