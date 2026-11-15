set(reqs esp_wifi_sensing
         esp-radar
         protocol_examples_common
         driver
         esp_driver_ledc
         led_strip)

idf_component_register(SRCS "app_main.c"
                            "led_control.c"
                            "csi_udp_broadcast.c"
                       INCLUDE_DIRS "."
                       REQUIRES ${reqs}
                       PRIV_REQUIRES esp_driver_usb_serial_jtag)
