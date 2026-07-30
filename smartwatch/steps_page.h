#ifndef SMARTWATCH_STEPS_PAGE_H
#define SMARTWATCH_STEPS_PAGE_H

#include "lvgl.h"
#include "smartwatch/watch_theme.h"

typedef struct {
    lv_obj_t *value_label;
    lv_obj_t *progress_bar;
} steps_page_t;

void steps_page_create(steps_page_t *page,
                       lv_obj_t *parent,
                       const watch_theme_t *theme);

void steps_page_set_value(steps_page_t *page,
                          uint16_t steps,
                          bool animate);

#endif
