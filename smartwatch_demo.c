#include "smartwatch_demo.h"

#include "smartwatch/calculator_page.h"
#include "smartwatch/clock_page.h"
#include "smartwatch/heart_page.h"
#include "smartwatch/settings_page.h"
#include "smartwatch/steps_page.h"

#include <string.h>

typedef struct {
    lv_obj_t *tabview;
    lv_timer_t *update_timer;
    clock_page_t clock_page;
    steps_page_t steps_page;
    heart_page_t heart_page;
    settings_page_t settings_page;
    calculator_page_t calculator_page;
    watch_theme_t theme;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t steps;
    uint8_t heart_rate;
    bool dark_mode;
} smartwatch_app_t;

static smartwatch_app_t watch;

static watch_theme_t make_theme(bool dark_mode)
{
    watch_theme_t theme;

    theme.background = dark_mode ? lv_color_hex(0x10141c)
                                 : lv_color_hex(0xeef2f7);
    theme.card = dark_mode ? lv_color_hex(0x202733)
                           : lv_color_hex(0xffffff);
    theme.text = dark_mode ? lv_color_hex(0xf4f7fb)
                           : lv_color_hex(0x18202b);
    theme.accent = lv_color_hex(0x45d4a8);
    theme.heart = lv_color_hex(0xff5b6e);
    return theme;
}

static void apply_theme(smartwatch_app_t *app)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_t *tab_bar = lv_tabview_get_tab_bar(app->tabview);

    lv_obj_set_style_bg_color(screen, app->theme.background, LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, app->theme.text, LV_PART_MAIN);
    lv_obj_set_style_bg_color(app->tabview, app->theme.background,
                              LV_PART_MAIN);
    lv_obj_set_style_text_color(app->tabview, app->theme.text, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_bar, app->theme.card, LV_PART_MAIN);

    settings_page_apply_theme(&app->settings_page, &app->theme);
}

static lv_obj_t *create_tab(lv_obj_t *tabview, const char *name)
{
    lv_obj_t *page = lv_tabview_add_tab(tabview, name);

    lv_obj_set_style_pad_all(page, 8, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return page;
}

static void close_calculator_cb(void *user_data)
{
    smartwatch_app_t *app = (smartwatch_app_t *)user_data;

    lv_obj_remove_flag(app->tabview, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(app->tabview);
    calculator_page_close(&app->calculator_page);
}

static void open_calculator_cb(void *user_data)
{
    smartwatch_app_t *app = (smartwatch_app_t *)user_data;

    if(calculator_page_is_open(&app->calculator_page)) return;

    lv_obj_add_flag(app->tabview, LV_OBJ_FLAG_HIDDEN);
    calculator_page_open(&app->calculator_page,
                         lv_screen_active(),
                         &app->theme,
                         close_calculator_cb,
                         app);
}

static void theme_changed_cb(bool dark_mode, void *user_data)
{
    smartwatch_app_t *app = (smartwatch_app_t *)user_data;

    app->dark_mode = dark_mode;
    app->theme = make_theme(dark_mode);
    apply_theme(app);
}

static void update_timer_cb(lv_timer_t *timer)
{
    smartwatch_app_t *app = (smartwatch_app_t *)lv_timer_get_user_data(timer);

    app->second++;
    if(app->second >= 60) {
        app->second = 0;
        app->minute++;
    }
    if(app->minute >= 60) {
        app->minute = 0;
        app->hour = (app->hour + 1) % 24;
    }

    app->steps = (app->steps + 3) % 10001;
    app->heart_rate = 62 + (app->second % 7);

    clock_page_set_time(&app->clock_page,
                        app->hour, app->minute, app->second);
    steps_page_set_value(&app->steps_page, app->steps, true);
    heart_page_set_rate(&app->heart_page, app->heart_rate);
}

void smartwatch_demo_create(void)
{
    lv_obj_t *clock_tab;
    lv_obj_t *steps_tab;
    lv_obj_t *heart_tab;
    lv_obj_t *settings_tab;

    if(watch.update_timer != NULL) lv_timer_delete(watch.update_timer);
    lv_obj_clean(lv_screen_active());
    memset(&watch, 0, sizeof(watch));

    watch.hour = 3;
    watch.minute = 20;
    watch.second = 15;
    watch.steps = 785;
    watch.heart_rate = 62;
    watch.dark_mode = true;
    watch.theme = make_theme(watch.dark_mode);

    watch.tabview = lv_tabview_create(lv_screen_active());
    lv_obj_set_size(watch.tabview, lv_pct(100), lv_pct(100));
    lv_tabview_set_tab_bar_position(watch.tabview, LV_DIR_BOTTOM);
    lv_tabview_set_tab_bar_size(watch.tabview, 58);

    clock_tab = create_tab(watch.tabview, "TIME");
    steps_tab = create_tab(watch.tabview, "STEPS");
    heart_tab = create_tab(watch.tabview, "HEART");
    settings_tab = create_tab(watch.tabview, "SET");
    lv_obj_set_flex_align(settings_tab, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    clock_page_create(&watch.clock_page, clock_tab, &watch.theme,
                      open_calculator_cb, &watch);
    steps_page_create(&watch.steps_page, steps_tab, &watch.theme);
    heart_page_create(&watch.heart_page, heart_tab, &watch.theme);
    settings_page_create(&watch.settings_page, settings_tab, &watch.theme,
                         watch.dark_mode, theme_changed_cb, &watch);

    clock_page_set_time(&watch.clock_page,
                        watch.hour, watch.minute, watch.second);
    steps_page_set_value(&watch.steps_page, watch.steps, true);
    heart_page_set_rate(&watch.heart_page, watch.heart_rate);
    apply_theme(&watch);

    watch.update_timer = lv_timer_create(update_timer_cb, 5000, &watch);
}
