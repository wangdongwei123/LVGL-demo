#ifndef SMARTWATCH_WATCH_THEME_H
#define SMARTWATCH_WATCH_THEME_H

#include "lvgl.h"

/* Shared visual values passed into modules by the application coordinator. */
typedef struct {
    lv_color_t background;
    lv_color_t card;
    lv_color_t text;
    lv_color_t accent;
    lv_color_t heart;
} watch_theme_t;

#endif
