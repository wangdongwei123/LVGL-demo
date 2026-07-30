#include "smartwatch/clock_page.h"

static void calculator_button_event_cb(lv_event_t *event)
{
    clock_page_t *page = (clock_page_t *)lv_event_get_user_data(event);

    if(page->open_calculator_cb != NULL) {
        page->open_calculator_cb(page->callback_user_data);
    }
}

void clock_page_create(clock_page_t *page,
                       lv_obj_t *parent,
                       const watch_theme_t *theme,
                       clock_page_open_calculator_cb_t open_calculator_cb,
                       void *user_data)
{
    lv_obj_t *title;
    lv_obj_t *hello;
    lv_obj_t *calculator_button;
    lv_obj_t *calculator_label;

    page->open_calculator_cb = open_calculator_cb;
    page->callback_user_data = user_data;

    title = lv_label_create(parent);
    lv_label_set_text(title, "WED 23 JUL");
    lv_obj_set_style_text_color(title, theme->accent, 0);

    page->time_label = lv_label_create(parent);
    lv_obj_set_style_text_font(page->time_label, &lv_font_montserrat_48, 0);

    page->seconds_label = lv_label_create(parent);

    hello = lv_label_create(parent);
    lv_label_set_text(hello, "Hello LVGL");

    calculator_button = lv_button_create(parent);
    lv_obj_set_size(calculator_button, 150, 36);
    lv_obj_add_event_cb(calculator_button, calculator_button_event_cb,
                        LV_EVENT_CLICKED, page);

    calculator_label = lv_label_create(calculator_button);
    lv_label_set_text(calculator_label, "CALCULATOR");
    lv_obj_center(calculator_label);
}

void clock_page_set_time(clock_page_t *page,
                         uint8_t hour,
                         uint8_t minute,
                         uint8_t second)
{
    lv_label_set_text_fmt(page->time_label, "%02u:%02u", hour, minute);
    lv_label_set_text_fmt(page->seconds_label, "%02u sec", second);
}
