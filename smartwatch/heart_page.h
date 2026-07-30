#ifndef SMARTWATCH_HEART_PAGE_H
#define SMARTWATCH_HEART_PAGE_H

#include "lvgl.h"
#include "smartwatch/watch_theme.h"

typedef struct {
    lv_obj_t *arc;
    lv_obj_t *value_label;
} heart_page_t;

void heart_page_create(heart_page_t *page,
                       lv_obj_t *parent,
                       const watch_theme_t *theme);

void heart_page_set_rate(heart_page_t *page, uint8_t heart_rate);

#endif
