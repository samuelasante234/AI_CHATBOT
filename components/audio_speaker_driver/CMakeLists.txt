idf_component_register(
    SRCS "audio_speaker_driver.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2s driver
)