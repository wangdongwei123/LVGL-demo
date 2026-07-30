#include "smartwatch/steps_page.h"

void steps_page_create(steps_page_t *page,
                       lv_obj_t *parent,
                       const watch_theme_t *theme)
{
    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "DAILY STEPS");
    lv_obj_set_style_text_color(title, theme->accent, 0);

    page->value_label = lv_label_create(parent);
    lv_obj_set_style_text_font(page->value_label, &lv_font_montserrat_24, 0);

    page->progress_bar = lv_bar_create(parent);
    lv_obj_set_size(page->progress_bar, 230, 22);
    lv_bar_set_range(page->progress_bar, 0, 10000);
    lv_obj_set_style_bg_color(page->progress_bar, theme->accent,
                              LV_PART_INDICATOR);

    lv_obj_t *hint = lv_label_create(parent);
    lv_label_set_text(hint, "Goal: keep moving!");
}

void steps_page_set_value(steps_page_t *page,
                          uint16_t steps,
                          bool animate)
{
    lv_bar_set_value(page->progress_bar, steps,
                     animate ? LV_ANIM_ON : LV_ANIM_OFF);
    lv_label_set_text_fmt(page->value_label, "%u / 10000", steps);
}
