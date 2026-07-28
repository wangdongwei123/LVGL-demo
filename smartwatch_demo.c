#include "smartwatch_demo.h"

typedef struct {
    lv_obj_t *time_label;
    lv_obj_t *seconds_label;
    lv_obj_t *steps_bar;
    lv_obj_t *steps_label;
    lv_obj_t *heart_arc;
    lv_obj_t *heart_label;
    lv_obj_t *tabview;
    lv_obj_t *settings_row;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t steps;
    uint8_t heart_rate;
    bool dark_mode;
} watch_state_t;

static watch_state_t watch;

#define COLOR_BG_DARK    lv_color_hex(0x10141c)
#define COLOR_BG_LIGHT   lv_color_hex(0xeef2f7)
#define COLOR_CARD_DARK  lv_color_hex(0x202733)
#define COLOR_CARD_LIGHT lv_color_hex(0xffffff)
#define COLOR_TEXT_DARK  lv_color_hex(0xf4f7fb)
#define COLOR_TEXT_LIGHT lv_color_hex(0x18202b)
#define COLOR_ACCENT     lv_color_hex(0x45d4a8)
#define COLOR_HEART      lv_color_hex(0xff5b6e)

static void apply_theme(void)
{
    lv_obj_t *screen = lv_screen_active();
    lv_color_t bg = watch.dark_mode ? COLOR_BG_DARK : COLOR_BG_LIGHT;
    lv_color_t card = watch.dark_mode ? COLOR_CARD_DARK : COLOR_CARD_LIGHT;
    lv_color_t text = watch.dark_mode ? COLOR_TEXT_DARK : COLOR_TEXT_LIGHT;

    lv_obj_set_style_bg_color(screen, bg, LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, text, LV_PART_MAIN);
    lv_obj_set_style_bg_color(watch.tabview, bg, LV_PART_MAIN);
    lv_obj_set_style_text_color(watch.tabview, text, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_tabview_get_tab_bar(watch.tabview), card,
                              LV_PART_MAIN);
    lv_obj_set_style_bg_color(watch.settings_row, card, LV_PART_MAIN);
}

static void tick_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    watch.second++;
    if(watch.second >= 60) {
        watch.second = 0;
        watch.minute++;
    }
    if(watch.minute >= 60) {
        watch.minute = 0;
        watch.hour = (watch.hour + 1) % 24;
    }

    watch.steps = (watch.steps + 3) % 10001;
    watch.heart_rate = 72 + (watch.second % 7);

    lv_label_set_text_fmt(watch.time_label, "%02u:%02u",
                          watch.hour, watch.minute);
    lv_label_set_text_fmt(watch.seconds_label, "%02u sec", watch.second);
    lv_bar_set_value(watch.steps_bar, watch.steps, LV_ANIM_ON);
    lv_label_set_text_fmt(watch.steps_label, "%u / 10000", watch.steps);
    lv_arc_set_value(watch.heart_arc, watch.heart_rate);
    lv_label_set_text_fmt(watch.heart_label, "%u\nbpm", watch.heart_rate);
}

static void theme_event_cb(lv_event_t *event)
{
    lv_obj_t *sw = lv_event_get_target_obj(event);
    watch.dark_mode = lv_obj_has_state(sw, LV_STATE_CHECKED);
    apply_theme();
}

static lv_obj_t *make_page(lv_obj_t *tabview, const char *name)
{
    lv_obj_t *page = lv_tabview_add_tab(tabview, name);
    lv_obj_set_style_pad_all(page, 8, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return page;
}

static void create_clock_page(lv_obj_t *tabview)
{
    lv_obj_t *page = make_page(tabview, "TIME");

    lv_obj_t *title = lv_label_create(page);
    lv_label_set_text(title, "WED 23 JUL");
    lv_obj_set_style_text_color(title, COLOR_ACCENT, 0);

    watch.time_label = lv_label_create(page);
    lv_label_set_text(watch.time_label, "03:20");
    lv_obj_set_style_text_font(watch.time_label, &lv_font_montserrat_48, 0);

    watch.seconds_label = lv_label_create(page);
    lv_label_set_text(watch.seconds_label, "15 sec");
}

static void create_steps_page(lv_obj_t *tabview)
{
    lv_obj_t *page = make_page(tabview, "STEPS");

    lv_obj_t *title = lv_label_create(page);
    lv_label_set_text(title, "DAILY STEPS");
    lv_obj_set_style_text_color(title, COLOR_ACCENT, 0);

    watch.steps_label = lv_label_create(page);
    lv_label_set_text(watch.steps_label, "2000 / 10000");
    lv_obj_set_style_text_font(watch.steps_label, &lv_font_montserrat_24, 0);

    watch.steps_bar = lv_bar_create(page);
    lv_obj_set_size(watch.steps_bar, 230, 22);
    lv_bar_set_range(watch.steps_bar, 0, 10000);
    lv_bar_set_value(watch.steps_bar, 2000, LV_ANIM_ON);
    lv_obj_set_style_bg_color(watch.steps_bar, COLOR_ACCENT, LV_PART_INDICATOR);

    lv_obj_t *hint = lv_label_create(page);
    lv_label_set_text(hint, "Goal: keep moving!");
}

static void create_heart_page(lv_obj_t *tabview)
{
    lv_obj_t *page = make_page(tabview, "HEART");

    watch.heart_arc = lv_arc_create(page);
    lv_obj_set_size(watch.heart_arc, 165, 165);
    lv_arc_set_range(watch.heart_arc, 50, 130);
    lv_arc_set_value(watch.heart_arc, 62);
    lv_obj_remove_flag(watch.heart_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(watch.heart_arc, COLOR_HEART, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(watch.heart_arc, 14, LV_PART_INDICATOR);

    watch.heart_label = lv_label_create(watch.heart_arc);
    lv_label_set_text(watch.heart_label, "62\nbpm");
    lv_obj_set_style_text_align(watch.heart_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(watch.heart_label, &lv_font_montserrat_24, 0);
    lv_obj_center(watch.heart_label);
}

static void create_settings_page(lv_obj_t *tabview)
{
    lv_obj_t *page = make_page(tabview, "SET");
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(page);
    lv_label_set_text(title, "SETTINGS");
    lv_obj_set_style_text_color(title, COLOR_ACCENT, 0);

    lv_obj_t *row = lv_obj_create(page);
    watch.settings_row = row;
    lv_obj_set_size(row, lv_pct(100), 58);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_radius(row, 14, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *label = lv_label_create(row);
    lv_label_set_text(label, "Dark mode");

    lv_obj_t *sw = lv_switch_create(row);
    //lv_obj_add_state(sw, LV_STATE_CHECKED);
    if(watch.dark_mode) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(sw, theme_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *brightness = lv_label_create(page);
    lv_label_set_text(brightness, "Brightness (UI only)");

    lv_obj_t *slider = lv_slider_create(page);
    lv_obj_set_width(slider, lv_pct(90));
    lv_slider_set_value(slider, 70, LV_ANIM_ON);
}

void smartwatch_demo_create(void)
{
    lv_obj_clean(lv_screen_active());

    watch.hour = 3;
    watch.minute = 20;
    watch.second = 15;
    watch.steps = 785;
    watch.heart_rate = 62;
    watch.dark_mode = false;

    lv_obj_t *tabview = lv_tabview_create(lv_screen_active());
    watch.tabview = tabview;
    lv_obj_set_size(tabview, lv_pct(100), lv_pct(100));
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_BOTTOM);
    lv_tabview_set_tab_bar_size(tabview, 58);

    create_clock_page(tabview);
    create_steps_page(tabview);
    create_heart_page(tabview);
    create_settings_page(tabview);

    apply_theme();
    lv_timer_create(tick_cb, 5000, NULL);
}
