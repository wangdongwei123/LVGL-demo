#ifndef SMARTWATCH_SETTINGS_PAGE_H
#define SMARTWATCH_SETTINGS_PAGE_H

#include "lvgl.h"
#include "smartwatch/watch_theme.h"

typedef void (*settings_page_theme_changed_cb_t)(bool dark_mode,
                                                  void *user_data);

typedef struct {
    lv_obj_t *card;
    lv_obj_t *brightness_label;
    bool dark_mode;
    settings_page_theme_changed_cb_t theme_changed_cb;
    void *callback_user_data;
} settings_page_t;

void settings_page_create(settings_page_t *page,
                          lv_obj_t *parent,
                          const watch_theme_t *theme,
                          bool dark_mode,
                          settings_page_theme_changed_cb_t theme_changed_cb,
                          void *user_data);

void settings_page_apply_theme(settings_page_t *page,
                               const watch_theme_t *theme);

#endif
