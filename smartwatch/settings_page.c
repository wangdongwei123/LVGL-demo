#include "smartwatch/settings_page.h"

static void theme_event_cb(lv_event_t *event)
{
    settings_page_t *page = (settings_page_t *)lv_event_get_user_data(event);
    lv_obj_t *sw = lv_event_get_target_obj(event);

    page->dark_mode = lv_obj_has_state(sw, LV_STATE_CHECKED);
    if(page->theme_changed_cb != NULL) {
        page->theme_changed_cb(page->dark_mode, page->callback_user_data);
    }
}

static void brightness_event_cb(lv_event_t *event)
{
    settings_page_t *page = (settings_page_t *)lv_event_get_user_data(event);
    lv_obj_t *slider = lv_event_get_target_obj(event);
    int32_t value = lv_slider_get_value(slider);

    lv_label_set_text_fmt(page->brightness_label,
                          "Brightness: %d%%", (int)value);
}

void settings_page_create(settings_page_t *page,
                          lv_obj_t *parent,
                          const watch_theme_t *theme,
                          bool dark_mode,
                          settings_page_theme_changed_cb_t theme_changed_cb,
                          void *user_data)
{
    lv_obj_t *title;
    lv_obj_t *label;
    lv_obj_t *sw;
    lv_obj_t *slider;

    page->dark_mode = dark_mode;
    page->theme_changed_cb = theme_changed_cb;
    page->callback_user_data = user_data;

    title = lv_label_create(parent);
    lv_label_set_text(title, "SETTINGS");
    lv_obj_set_style_text_color(title, theme->accent, 0);

    page->card = lv_obj_create(parent);
    lv_obj_set_size(page->card, lv_pct(100), 58);
    lv_obj_set_style_border_width(page->card, 0, 0);
    lv_obj_set_style_radius(page->card, 14, 0);
    lv_obj_set_flex_flow(page->card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(page->card, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(page->card);
    lv_label_set_text(label, "Dark mode");

    sw = lv_switch_create(page->card);
    if(dark_mode) lv_obj_add_state(sw, LV_STATE_CHECKED);
    else lv_obj_clear_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, theme_event_cb, LV_EVENT_VALUE_CHANGED, page);

    page->brightness_label = lv_label_create(parent);
    lv_label_set_text(page->brightness_label, "Brightness: 70%");

    slider = lv_slider_create(parent);
    lv_obj_set_width(slider, lv_pct(90));
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 70, LV_ANIM_ON);
    lv_obj_add_event_cb(slider, brightness_event_cb,
                        LV_EVENT_VALUE_CHANGED, page);

    settings_page_apply_theme(page, theme);
}

void settings_page_apply_theme(settings_page_t *page,
                               const watch_theme_t *theme)
{
    lv_obj_set_style_bg_color(page->card, theme->card, LV_PART_MAIN);
}
