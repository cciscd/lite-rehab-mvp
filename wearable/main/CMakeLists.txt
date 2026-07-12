idf_component_register(
    SRCS "app_main.c" "mpu6050.c" "ssd1306.c" "ble_server.c" "wearable_status.c"
         "../../shared/motion_packet.c" "../../shared/motion_logic.c"
    INCLUDE_DIRS "." "../../shared"
    PRIV_REQUIRES esp_driver_i2c esp_driver_gpio bt nvs_flash
)
