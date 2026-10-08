idf_component_register(
  SRCS "src/bidi_switch_knob.c" "user_encoder_bsp.c"
  REQUIRES hw_config esp_driver_gpio
  INCLUDE_DIRS "./" "./src")
