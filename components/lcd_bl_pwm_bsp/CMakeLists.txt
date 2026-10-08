set(component_requires driver hw_config)
if(IDF_VERSION_MAJOR GREATER_EQUAL 6)
  list(APPEND component_requires esp_driver_gpio esp_driver_ledc)
endif()

idf_component_register(
  SRCS "lcd_bl_pwm_bsp.c"
  REQUIRES ${component_requires}
  INCLUDE_DIRS "./")
