#include "UiTheme.h"
#include <lvgl.h>

using namespace VoxUiTheme;

static lv_style_t style_button;
static lv_style_t style_card;
static lv_style_t style_header;
static lv_style_t style_meter;
static lv_style_t style_text_primary;
static lv_style_t style_text_secondary;

void ui_theme_init(void) {
    // Button style - large touch target
    lv_style_init(&style_button);
    lv_style_set_bg_color(&style_button, COLOR_ACCENT);
    lv_style_set_bg_opa(&style_button, LV_OPA_COVER);
    lv_style_set_radius(&style_button, 8);
    lv_style_set_pad_all(&style_button, 12);
    lv_style_set_pad_row(&style_button, 8);
    lv_style_set_text_color(&style_button, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_button, &lv_font_montserrat_16);

    // Card style
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_BG_CARD);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 8);
    lv_style_set_pad_all(&style_card, 12);
    lv_style_set_border_width(&style_card, 0);

    // Header style
    lv_style_init(&style_header);
    lv_style_set_bg_color(&style_header, COLOR_BG_HEADER);
    lv_style_set_bg_opa(&style_header, LV_OPA_COVER);
    lv_style_set_radius(&style_header, 0);
    lv_style_set_pad_hor(&style_header, 8);
    lv_style_set_pad_ver(&style_header, 6);

    // Meter style
    lv_style_init(&style_meter);
    lv_style_set_bg_color(&style_meter, lv_color_make(0x30, 0x30, 0x30));
    lv_style_set_bg_opa(&style_meter, LV_OPA_COVER);
    lv_style_set_radius(&style_meter, 4);

    // Primary text style
    lv_style_init(&style_text_primary);
    lv_style_set_text_color(&style_text_primary, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_text_primary, &lv_font_montserrat_14);

    // Secondary text style
    lv_style_init(&style_text_secondary);
    lv_style_set_text_color(&style_text_secondary, COLOR_TEXT_SECONDARY);
    lv_style_set_text_font(&style_text_secondary, &lv_font_montserrat_14);
}

lv_style_t* ui_style_get_button(void) {
    return &style_button;
}

lv_style_t* ui_style_get_card(void) {
    return &style_card;
}

lv_style_t* ui_style_get_header(void) {
    return &style_header;
}

lv_style_t* ui_style_get_meter(void) {
    return &style_meter;
}

lv_style_t* ui_style_get_text_primary(void) {
    return &style_text_primary;
}

lv_style_t* ui_style_get_text_secondary(void) {
    return &style_text_secondary;
}
