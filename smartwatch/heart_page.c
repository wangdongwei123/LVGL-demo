#include "smartwatch/heart_page.h"

void heart_page_create(heart_page_t *page,
                       lv_obj_t *parent,
                       const watch_theme_t *theme)
{
    page->arc = lv_arc_create(parent);
    lv_obj_set_size(page->arc, 165, 165);
    lv_arc_set_range(page->arc, 50, 130);
    lv_obj_remove_flag(page->arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(page->arc, theme->heart, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(page->arc, 14, LV_PART_INDICATOR);

    page->value_label = lv_label_create(page->arc);
    lv_obj_set_style_text_align(page->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(page->value_label, &lv_font_montserrat_24, 0);
    lv_obj_center(page->value_label);
}

void heart_page_set_rate(heart_page_t *page, uint8_t heart_rate)
{
    lv_arc_set_value(page->arc, heart_rate);
    lv_label_set_text_fmt(page->value_label, "%u\nbpm", heart_rate);
}
