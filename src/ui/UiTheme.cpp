#include "UiTheme.h"
#include <lvgl.h>

namespace VoxUiTheme {

static lv_style_t style_button;
static lv_style_t style_card;
static lv_style_t style_header;
static lv_style_t style_meter;
static lv_style_t style_text_primary;
static lv_style_t style_text_secondary;

}  // namespace VoxUiTheme

void ui_theme_init(void) {
    using namespace VoxUiTheme;

    // Button style - large touch target
    lv_style_init(&style_button);
    lv_style_set_bg_color(&style_button, lv_color_make(COLOR_ACCENT.ch.red, COLOR_ACCENT.ch.green, COLOR_ACCENT.ch.blue));
    lv_style_set_bg_opa(&style_button, LV_OPA_COVER);
    lv_style_set_radius(&style_button, 8);
    lv_style_set_pad_all(&style_button, 12);
    lv_style_set_pad_row(&style_button, 8);
    lv_style_set_text_color(&style_button, lv_color_make(COLOR_TEXT_PRIMARY.ch.red, COLOR_TEXT_PRIMARY.ch.green, COLOR_TEXT_PRIMARY.ch.blue));
    lv_style_set_text_font(&style_button, &lv_font_montserrat_16);

    // Card style
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_make(COLOR_BG_CARD.ch.red, COLOR_BG_CARD.ch.green, COLOR_BG_CARD.ch.blue));
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 8);
    lv_style_set_pad_all(&style_card, 12);
    lv_style_set_border_width(&style_card, 0);

    // Header style
    lv_style_init(&style_header);
    lv_style_set_bg_color(&style_header, lv_color_make(COLOR_BG_HEADER.ch.red, COLOR_BG_HEADER.ch.green, COLOR_BG_HEADER.ch.blue));
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
    lv_style_set_text_color(&style_text_primary, lv_color_make(COLOR_TEXT_PRIMARY.ch.red, COLOR_TEXT_PRIMARY.ch.green, COLOR_TEXT_PRIMARY.ch.blue));
    lv_style_set_text_font(&style_text_primary, &lv_font_montserrat_14);

    // Secondary text style
    lv_style_init(&style_text_secondary);
    lv_style_set_text_color(&style_text_secondary, lv_color_make(COLOR_TEXT_SECONDARY.ch.red, COLOR_TEXT_SECONDARY.ch.green, COLOR_TEXT_SECONDARY.ch.blue));
    lv_style_set_text_font(&style_text_secondary, &lv_font_montserrat_12);
}

lv_style_t* ui_style_get_button(void) {
    return &VoxUiTheme::style_button;
}

lv_style_t* ui_style_get_card(void) {
    return &VoxUiTheme::style_card;
}

lv_style_t* ui_style_get_header(void) {
    return &VoxUiTheme::style_header;
}

lv_style_t* ui_style_get_meter(void) {
    return &VoxUiTheme::style_meter;
}

lv_style_t* ui_style_get_text_primary(void) {
    return &VoxUiTheme::style_text_primary;
}

lv_style_t* ui_style_get_text_secondary(void) {
    return &VoxUiTheme::style_text_secondary;
}
