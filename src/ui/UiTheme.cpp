#include "UiTheme.h"
#include <lvgl.h>

using namespace VoxUiTheme;

// Static style instances
static lv_style_t style_bg;
static lv_style_t style_surface;
static lv_style_t style_elevated;
static lv_style_t style_card;
static lv_style_t style_header;
static lv_style_t style_meter;
static lv_style_t style_text_primary;
static lv_style_t style_text_secondary;
static lv_style_t style_text_muted;
static lv_style_t style_button;
static lv_style_t style_nav_tab;

void ui_theme_init(void) {
    // Background style - main app background
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, COLOR_BG_DARK);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
    
    // Surface style - card/container background
    lv_style_init(&style_surface);
    lv_style_set_bg_color(&style_surface, COLOR_BG_SURFACE);
    lv_style_set_bg_opa(&style_surface, LV_OPA_COVER);
    lv_style_set_radius(&style_surface, RADIUS_M);
    lv_style_set_pad_all(&style_surface, SPACING_M);
    lv_style_set_border_width(&style_surface, 0);
    
    // Elevated surface style
    lv_style_init(&style_elevated);
    lv_style_set_bg_color(&style_elevated, COLOR_BG_ELEVATED);
    lv_style_set_bg_opa(&style_elevated, LV_OPA_COVER);
    lv_style_set_radius(&style_elevated, RADIUS_M);
    lv_style_set_pad_all(&style_elevated, SPACING_M);
    lv_style_set_border_width(&style_elevated, 1);
    lv_style_set_border_color(&style_elevated, COLOR_BORDER_SUBTLE);
    
    // Card style - compact container
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_BG_SURFACE);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, RADIUS_M);
    lv_style_set_pad_all(&style_card, SPACING_S);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, COLOR_BORDER_SUBTLE);
    
    // Header style - top bar
    lv_style_init(&style_header);
    lv_style_set_bg_color(&style_header, COLOR_BG_HEADER);
    lv_style_set_bg_opa(&style_header, LV_OPA_COVER);
    lv_style_set_radius(&style_header, 0);
    lv_style_set_pad_hor(&style_header, SPACING_M);
    lv_style_set_pad_ver(&style_header, SPACING_S);
    lv_style_set_border_width(&style_header, 0);
    
    // Meter style - VU meter background
    lv_style_init(&style_meter);
    lv_style_set_bg_color(&style_meter, lv_color_make(0x1A, 0x1F, 0x2A));
    lv_style_set_bg_opa(&style_meter, LV_OPA_COVER);
    lv_style_set_radius(&style_meter, RADIUS_S);
    lv_style_set_border_width(&style_meter, 0);
    
    // Primary text style - main labels
    lv_style_init(&style_text_primary);
    lv_style_set_text_color(&style_text_primary, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_text_primary, &lv_font_montserrat_14);
    
    // Secondary text style - supporting labels
    lv_style_init(&style_text_secondary);
    lv_style_set_text_color(&style_text_secondary, COLOR_TEXT_SECONDARY);
    lv_style_set_text_font(&style_text_secondary, &lv_font_montserrat_12);
    
    // Muted text style - inactive/disabled
    lv_style_init(&style_text_muted);
    lv_style_set_text_color(&style_text_muted, COLOR_TEXT_MUTED);
    lv_style_set_text_font(&style_text_muted, &lv_font_montserrat_12);
    
    // Button style - large touch target
    lv_style_init(&style_button);
    lv_style_set_bg_color(&style_button, COLOR_ACCENT_PRIMARY);
    lv_style_set_bg_opa(&style_button, LV_OPA_COVER);
    lv_style_set_radius(&style_button, RADIUS_M);
    lv_style_set_pad_all(&style_button, SPACING_L);
    lv_style_set_text_color(&style_button, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_button, &lv_font_montserrat_16);
    
    // Navigation tab style
    lv_style_init(&style_nav_tab);
    lv_style_set_bg_color(&style_nav_tab, COLOR_BG_HEADER);
    lv_style_set_bg_opa(&style_nav_tab, LV_OPA_COVER);
    lv_style_set_radius(&style_nav_tab, 0);
    lv_style_set_pad_all(&style_nav_tab, SPACING_S);
    lv_style_set_border_width(&style_nav_tab, 0);
    lv_style_set_text_color(&style_nav_tab, COLOR_TEXT_SECONDARY);
    lv_style_set_text_font(&style_nav_tab, &lv_font_montserrat_12);
}

lv_color_t UiTheme_get_color(UiThemeColor color) {
    switch (color) {
        case THEME_COLOR_BG_DARK: return COLOR_BG_DARK;
        case THEME_COLOR_BG_SURFACE: return COLOR_BG_SURFACE;
        case THEME_COLOR_BG_ELEVATED: return COLOR_BG_ELEVATED;
        case THEME_COLOR_BG_HEADER: return COLOR_BG_HEADER;
        case THEME_COLOR_ACCENT_PRIMARY: return COLOR_ACCENT_PRIMARY;
        case THEME_COLOR_ACCENT_BRIGHT: return COLOR_ACCENT_BRIGHT;
        case THEME_COLOR_ACCENT_CYAN: return COLOR_ACCENT_CYAN;
        case THEME_COLOR_SUCCESS: return COLOR_SUCCESS;
        case THEME_COLOR_WARNING: return COLOR_WARNING;
        case THEME_COLOR_ERROR: return COLOR_ERROR;
        case THEME_COLOR_LINK_OK: return COLOR_LINK_OK;
        case THEME_COLOR_LINK_LOST: return COLOR_LINK_LOST;
        case THEME_COLOR_DISABLED: return COLOR_DISABLED;
        case THEME_COLOR_BORDER_SUBTLE: return COLOR_BORDER_SUBTLE;
        case THEME_COLOR_BORDER_ACTIVE: return COLOR_BORDER_ACTIVE;
        case THEME_COLOR_TEXT_PRIMARY: return COLOR_TEXT_PRIMARY;
        case THEME_COLOR_TEXT_SECONDARY: return COLOR_TEXT_SECONDARY;
        case THEME_COLOR_TEXT_MUTED: return COLOR_TEXT_MUTED;
        case THEME_COLOR_METER_SAFE: return COLOR_METER_SAFE;
        case THEME_COLOR_METER_WARNING: return COLOR_METER_WARNING;
        case THEME_COLOR_METER_CLIP: return COLOR_METER_CLIP;
        case THEME_COLOR_METER_PEAK: return COLOR_METER_PEAK;
        case THEME_COLOR_EFFECT_LED_OFF: return COLOR_EFFECT_LED_OFF;
        case THEME_COLOR_EFFECT_LED_ON: return COLOR_EFFECT_LED_ON;
        case THEME_COLOR_FS_PRESSED: return COLOR_FS_PRESSED;
        default: return COLOR_BG_DARK;
    }
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

lv_style_t* ui_style_get_text_muted(void) {
    return &style_text_muted;
}

lv_style_t* ui_style_get_button(void) {
    return &style_button;
}

lv_style_t* ui_style_get_nav_tab(void) {
    return &style_nav_tab;
}
