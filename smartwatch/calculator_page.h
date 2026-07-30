#ifndef SMARTWATCH_CALCULATOR_PAGE_H
#define SMARTWATCH_CALCULATOR_PAGE_H

#include "lvgl.h"
#include "smartwatch/watch_theme.h"

typedef void (*calculator_page_back_cb_t)(void *user_data);

typedef struct {
    lv_obj_t *container;
    lv_obj_t *expression_label;
    lv_obj_t *display_label;
    char input[24];
    char left_input[24];
    double accumulator;
    char pending_operator;
    bool new_input;
    bool error;
    calculator_page_back_cb_t back_cb;
    void *callback_user_data;
} calculator_page_t;

void calculator_page_open(calculator_page_t *page,
                          lv_obj_t *parent,
                          const watch_theme_t *theme,
                          calculator_page_back_cb_t back_cb,
                          void *user_data);

void calculator_page_close(calculator_page_t *page);
bool calculator_page_is_open(const calculator_page_t *page);

#endif
