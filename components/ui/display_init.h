#ifndef DISPLAY_INIT_H
#define DISPLAY_INIT_H

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool ui_lvgl_lock(int timeout_ms);
void ui_lvgl_unlock(void);
void display_init(void);
bool display_take_first_frame_ready(void);
bool display_take_touch_activity(void);
bool display_backlight_is_asleep(void);
void display_set_backlight_enabled(bool enabled);

#endif