#include "lvgl.h"
#include "smartwatch_demo.h"

/*
 * Put this call after lv_init() and after creating your display/input driver.
 * The display driver is platform-specific, so it is intentionally not hidden
 * inside the UI demo.
 */
void app_ui_start(void)
{
    smartwatch_demo_create();
}

/* Your main loop must continue calling LVGL, for example:
 *
 * while(1) {
 *     lv_timer_handler();
 *     platform_delay_ms(5);
 * }
 */
