#ifndef SMARTWATCH_CLOCK_PAGE_H
#define SMARTWATCH_CLOCK_PAGE_H

#include "lvgl.h"
#include "smartwatch/watch_theme.h"

typedef void (*clock_page_open_calculator_cb_t)(void *user_data);

typedef struct {
    lv_obj_t *time_label;
    lv_obj_t *seconds_label;
    clock_page_open_calculator_cb_t open_calculator_cb;
    void *callback_user_data;
} clock_page_t;

void clock_page_create(clock_page_t *page,
                       lv_obj_t *parent,
                       const watch_theme_t *theme,
                       clock_page_open_calculator_cb_t open_calculator_cb,
                       void *user_data);

void clock_page_set_time(clock_page_t *page,
                         uint8_t hour,
                         uint8_t minute,
                         uint8_t second);

#endif
