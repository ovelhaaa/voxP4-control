#include "UiTheme.h"
#include <lvgl.h>

using namespace VoxUiTheme;

// Static style instances, shared by the screens.
static lv_style_t style_bg;
static lv_style_t style_surface;
static lv_style_t style_elevated;
static lv_style_t style_header;
static lv_style_t style_text_primary;
static lv_style_t style_text_secondary;
static lv_style_t style_text_muted;

void ui_theme_init(void) {
    // Main app background
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, COLOR_BG);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
    lv_style_set_border_width(&style_bg, 0);
    lv_style_set_radius(&style_bg, 0);

    // Card / panel surface
    lv_style_init(&style_surface);
    lv_style_set_bg_color(&style_surface, COLOR_SURFACE);
    lv_style_set_bg_opa(&style_surface, LV_OPA_COVER);
    lv_style_set_radius(&style_surface, RADIUS_M);
    lv_style_set_pad_all(&style_surface, SPACING_S);
    lv_style_set_border_width(&style_surface, 0);

    // Elevated surface
    lv_style_init(&style_elevated);
    lv_style_set_bg_color(&style_elevated, COLOR_SURFACE_ELEV);
    lv_style_set_bg_opa(&style_elevated, LV_OPA_COVER);
    lv_style_set_radius(&style_elevated, RADIUS_M);
    lv_style_set_pad_all(&style_elevated, SPACING_S);
    lv_style_set_border_width(&style_elevated, 1);
    lv_style_set_border_color(&style_elevated, COLOR_SEPARATOR);

    // Top bar / navigation bar
    lv_style_init(&style_header);
    lv_style_set_bg_color(&style_header, COLOR_HEADER);
    lv_style_set_bg_opa(&style_header, LV_OPA_COVER);
    lv_style_set_radius(&style_header, 0);
    lv_style_set_pad_hor(&style_header, SPACING_M);
    lv_style_set_pad_ver(&style_header, SPACING_XS);
    lv_style_set_border_width(&style_header, 0);

    // Text styles
    lv_style_init(&style_text_primary);
    lv_style_set_text_color(&style_text_primary, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_text_primary, FONT_BODY);

    lv_style_init(&style_text_secondary);
    lv_style_set_text_color(&style_text_secondary, COLOR_TEXT_SECONDARY);
    lv_style_set_text_font(&style_text_secondary, FONT_SMALL);

    lv_style_init(&style_text_muted);
    lv_style_set_text_color(&style_text_muted, COLOR_TEXT_MUTED);
    lv_style_set_text_font(&style_text_muted, FONT_SMALL);
}

lv_style_t* ui_style_get_bg(void) {
    return &style_bg;
}

lv_style_t* ui_style_get_surface(void) {
    return &style_surface;
}

lv_style_t* ui_style_get_elevated(void) {
    return &style_elevated;
}

lv_style_t* ui_style_get_header(void) {
    return &style_header;
}

lv_style_t* ui_style_get_text_primary(void) {
    return &style_text_primary;
}

lv_style_t* ui_style_get_text_secondary(void) {
    return &style_text_secondary;
}

lv_style_t* ui_style_get_text_muted(void) {
    return &style_text_muted;
}

void ui_apply_pressed(lv_obj_t* obj, lv_color_t bg, lv_color_t border) {
    if (!obj) return;
    lv_obj_set_style_bg_color(obj, bg, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(obj, border, LV_STATE_PRESSED);
}

void ui_apply_disabled(lv_obj_t* obj) {
    if (!obj) return;

    // Not interactive at all.
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);

    // Neutral, low-contrast treatment.
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(obj, COLOR_DISABLED, 0);
    lv_obj_set_style_opa(obj, LV_OPA_50, 0);

    // Kill any pressed feedback that may have been registered earlier.
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(obj, COLOR_DISABLED, LV_STATE_PRESSED);

    // Mute direct child labels so the control reads as unavailable.
    uint32_t child_count = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t* child = lv_obj_get_child(obj, i);
        if (lv_obj_check_type(child, &lv_label_class)) {
            lv_obj_set_style_text_color(child, COLOR_TEXT_MUTED, 0);
        }
    }
}
