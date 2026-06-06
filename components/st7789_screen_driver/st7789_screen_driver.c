#include "st7789_screen_driver.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "soc/gpio_reg.h"

static void IRAM_ATTR ISR(void *arg);
spi_device_handle_t st7789_init();
void send_data(spi_device_handle_t dev_handle, const uint8_t* data, int len);
void send_command(spi_device_handle_t dev_handle, const uint8_t command);

spi_device_handle_t st7789_init() {
    spi_bus_config_t bus_conf = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num=-1,
        .sclk_io_num=SCK_PIN,
        .quadhd_io_num=-1,
        .quadwp_io_num=-1,
        .intr_flags=0,
        .max_transfer_sz=MAX_TRANSFER_SIZE,
    };
    esp_err_t output = spi_bus_initialize(SPI_CHAN, &bus_conf, SPI_DMA_CH_AUTO);
    if (output!=ESP_OK) {
        printf("Couldn't initialise bus. Error: %s\n", esp_err_to_name(output));
        fflush(stdout);
        return NULL;
    }
    spi_device_interface_config_t spi_conf={
        .clock_source=SPI_CLK_SRC_APB,
        .clock_speed_hz=CLOCK_SPEED,
        .duty_cycle_pos=DUTY_COUNT,
        .mode=CPOL_CPHA,
        .command_bits=0,
        .address_bits=0,
        .dummy_bits=0,
        .queue_size=1,
        .spics_io_num=CS_PIN,
        .cs_ena_pretrans=TCSS,
        .cs_ena_posttrans=TCSH,
        .pre_cb=(transaction_cb_t) ISR,
    };
    spi_device_handle_t dev_handle;
    output= spi_bus_add_device(SPI_CHAN, &spi_conf, &dev_handle);
    if (output != ESP_OK) {
        printf("Couldn't add device! Error : %s\n", esp_err_to_name(output));
        fflush(stdout);
        return NULL;
    }
    return dev_handle;
}

static void IRAM_ATTR ISR(void *arg)  {
    spi_transaction_t* flag = (spi_transaction_t *) arg;
    if (flag->user) {
        volatile uint32_t *Overhead = (volatile uint32_t *)GPIO_OUT_W1TS_REG;
        *Overhead = 1<<DC_PIN;
    }
    else {
        volatile uint32_t *Overhead = (volatile uint32_t *)GPIO_OUT_W1TC_REG;
        *Overhead = 1<<DC_PIN;
    }
}
void send_data(spi_device_handle_t dev_handle, const uint8_t* data, int len_bytes) {
    spi_transaction_t transact_t = {0};
    transact_t.length=len_bytes*8; transact_t.tx_buffer=data; transact_t.user=(void*)1;
    esp_err_t result = spi_device_polling_transmit(dev_handle,&transact_t);
    if (result != ESP_OK) {
        printf("Couldn't send data! Error: %s", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}
void send_command(spi_device_handle_t dev_handle, const uint8_t command) {
    spi_transaction_t transact_t ={0};
    transact_t.length=8; transact_t.tx_buffer=&command; transact_t.user=(void*)0;
    esp_err_t result = spi_device_polling_transmit(dev_handle, &transact_t);
    if (result != ESP_OK) {
        printf("Couldn't send command! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}