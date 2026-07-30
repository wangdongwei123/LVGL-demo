#include "smartwatch/calculator_page.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *calculator_key_map[] = {
    "C", "DEL", "/", "*", "\n",
    "7", "8", "9", "-", "\n",
    "4", "5", "6", "+", "\n",
    "1", "2", "3", "=", "\n",
    "0", ".", ""
};

static void update_display(calculator_page_t *page)
{
    lv_label_set_text(page->display_label, page->input);
}

static void update_expression(calculator_page_t *page,
                              bool include_right_value,
                              bool include_equals_sign)
{
    char expression[64];

    if(page->pending_operator == '\0') {
        lv_label_set_text(page->expression_label, "");
        return;
    }

    if(include_right_value) {
        snprintf(expression, sizeof(expression), "%s %c %s%s",
                 page->left_input,
                 page->pending_operator,
                 page->input,
                 include_equals_sign ? " =" : "");
    }
    else {
        snprintf(expression, sizeof(expression), "%s %c",
                 page->left_input,
                 page->pending_operator);
    }

    lv_label_set_text(page->expression_label, expression);
}

static void clear_calculation(calculator_page_t *page)
{
    strcpy(page->input, "0");
    page->left_input[0] = '\0';
    page->accumulator = 0.0;
    page->pending_operator = '\0';
    page->new_input = true;
    page->error = false;
    lv_label_set_text(page->expression_label, "");
    update_display(page);
}

static bool apply_operator(calculator_page_t *page, double right_value)
{
    switch(page->pending_operator) {
        case '+':
            page->accumulator += right_value;
            break;
        case '-':
            page->accumulator -= right_value;
            break;
        case '*':
            page->accumulator *= right_value;
            break;
        case '/':
            if(right_value == 0.0) {
                strcpy(page->input, "Error");
                page->error = true;
                update_display(page);
                return false;
            }
            page->accumulator /= right_value;
            break;
        default:
            page->accumulator = right_value;
            break;
    }

    snprintf(page->input, sizeof(page->input), "%.8g", page->accumulator);
    update_display(page);
    return true;
}

static void enter_digit(calculator_page_t *page, const char *key)
{
    size_t length;

    if(page->error) clear_calculation(page);

    if(page->new_input) {
        if(page->pending_operator == '\0') {
            lv_label_set_text(page->expression_label, "");
        }
        strcpy(page->input, strcmp(key, ".") == 0 ? "0." : key);
        page->new_input = false;
        update_display(page);
        if(page->pending_operator != '\0') {
            update_expression(page, true, false);
        }
        return;
    }

    if(strcmp(key, ".") == 0 && strchr(page->input, '.') != NULL) return;

    length = strlen(page->input);
    if(length + strlen(key) < sizeof(page->input)) {
        strcat(page->input, key);
        update_display(page);
        if(page->pending_operator != '\0') {
            update_expression(page, true, false);
        }
    }
}

static void back_button_event_cb(lv_event_t *event)
{
    calculator_page_t *page =
        (calculator_page_t *)lv_event_get_user_data(event);

    if(page->back_cb != NULL) page->back_cb(page->callback_user_data);
}

static void keypad_event_cb(lv_event_t *event)
{
    calculator_page_t *page =
        (calculator_page_t *)lv_event_get_user_data(event);
    lv_obj_t *keypad = lv_event_get_target_obj(event);
    uint32_t button_id = lv_buttonmatrix_get_selected_button(keypad);
    const char *key;
    double current_value;

    if(button_id == LV_BUTTONMATRIX_BUTTON_NONE) return;
    key = lv_buttonmatrix_get_button_text(keypad, button_id);
    if(key == NULL) return;

    if(strcmp(key, "C") == 0) {
        clear_calculation(page);
        return;
    }

    if(strcmp(key, "DEL") == 0) {
        size_t length = strlen(page->input);
        if(page->error) {
            clear_calculation(page);
        }
        else if(!page->new_input) {
            if(length <= 1) strcpy(page->input, "0");
            else page->input[length - 1] = '\0';

            update_display(page);
            if(page->pending_operator != '\0') {
                update_expression(page, true, false);
            }
        }
        return;
    }

    if((key[0] >= '0' && key[0] <= '9') || strcmp(key, ".") == 0) {
        enter_digit(page, key);
        return;
    }

    current_value = strtod(page->input, NULL);

    if(strcmp(key, "=") == 0) {
        if(page->pending_operator != '\0' && !page->new_input &&
           !page->error) {
            update_expression(page, true, true);
            if(apply_operator(page, current_value)) {
                page->pending_operator = '\0';
                page->new_input = true;
            }
        }
        return;
    }

    if(page->error) clear_calculation(page);
    if(page->pending_operator != '\0' && !page->new_input) {
        if(!apply_operator(page, current_value)) return;
    }
    else if(!page->new_input || page->pending_operator == '\0') {
        page->accumulator = current_value;
    }

    snprintf(page->left_input, sizeof(page->left_input), "%s", page->input);
    page->pending_operator = key[0];
    page->new_input = true;
    update_expression(page, false, false);
}

void calculator_page_open(calculator_page_t *page,
                          lv_obj_t *parent,
                          const watch_theme_t *theme,
                          calculator_page_back_cb_t back_cb,
                          void *user_data)
{
    lv_obj_t *back_button;
    lv_obj_t *back_label;
    lv_obj_t *title;
    lv_obj_t *display_card;
    lv_obj_t *keypad;

    if(calculator_page_is_open(page)) return;

    page->back_cb = back_cb;
    page->callback_user_data = user_data;
    page->container = lv_obj_create(parent);
    lv_obj_set_size(page->container, lv_pct(100), lv_pct(100));
    lv_obj_center(page->container);
    lv_obj_remove_flag(page->container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(page->container, 8, 0);
    lv_obj_set_style_border_width(page->container, 0, 0);
    lv_obj_set_style_radius(page->container, 0, 0);
    lv_obj_set_style_bg_color(page->container, theme->background, 0);
    lv_obj_set_style_text_color(page->container, theme->text, 0);

    back_button = lv_button_create(page->container);
    lv_obj_set_size(back_button, 70, 32);
    lv_obj_align(back_button, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_event_cb(back_button, back_button_event_cb,
                        LV_EVENT_CLICKED, page);

    back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "BACK");
    lv_obj_center(back_label);

    title = lv_label_create(page->container);
    lv_label_set_text(title, "CALCULATOR");
    lv_obj_set_style_text_color(title, theme->accent, 0);
    lv_obj_align(title, LV_ALIGN_TOP_RIGHT, 0, 8);

    display_card = lv_obj_create(page->container);
    lv_obj_set_size(display_card, lv_pct(100), 70);
    lv_obj_align(display_card, LV_ALIGN_TOP_MID, 0, 38);
    lv_obj_remove_flag(display_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(display_card, theme->card, 0);
    lv_obj_set_style_border_width(display_card, 0, 0);
    lv_obj_set_style_radius(display_card, 10, 0);
    lv_obj_set_style_pad_all(display_card, 8, 0);

    page->expression_label = lv_label_create(display_card);
    lv_obj_set_width(page->expression_label, lv_pct(100));
    lv_obj_set_style_text_align(page->expression_label,
                                LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(page->expression_label, theme->accent, 0);
    lv_obj_align(page->expression_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    page->display_label = lv_label_create(display_card);
    lv_obj_set_width(page->display_label, lv_pct(100));
    lv_obj_set_style_text_align(page->display_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(page->display_label,
                               &lv_font_montserrat_24, 0);
    lv_obj_align(page->display_label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    keypad = lv_buttonmatrix_create(page->container);
    lv_buttonmatrix_set_map(keypad, calculator_key_map);
    lv_obj_set_size(keypad, lv_pct(100), 186);
    lv_obj_align(keypad, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(keypad, theme->background, LV_PART_MAIN);
    lv_obj_set_style_bg_color(keypad, theme->card, LV_PART_ITEMS);
    lv_obj_set_style_text_color(keypad, theme->text, LV_PART_ITEMS);
    lv_obj_set_style_border_width(keypad, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(keypad, keypad_event_cb,
                        LV_EVENT_VALUE_CHANGED, page);

    clear_calculation(page);
}

void calculator_page_close(calculator_page_t *page)
{
    lv_obj_t *container;

    if(!calculator_page_is_open(page)) return;

    container = page->container;
    page->container = NULL;
    page->expression_label = NULL;
    page->display_label = NULL;
    lv_obj_delete_async(container);
}

bool calculator_page_is_open(const calculator_page_t *page)
{
    return page->container != NULL;
}
