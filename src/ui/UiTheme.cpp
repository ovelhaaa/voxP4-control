#include "UiTheme.h"
#include <lvgl.h>

using namespace VoxUiTheme;

/**
 * Design System Implementation
 * 
 * Estilos centralizados para consistência visual e manutenção.
 * Todos os estilos usam as cores do theme e seguem a hierarquia definida.
 */

// Background styles
static lv_style_t style_bg;
static lv_style_t style_surface;
static lv_style_t style_surface_elevated;

// Card styles
static lv_style_t style_card;
static lv_style_t style_card_active;

// Container styles
static lv_style_t style_header;
static lv_style_t style_navbar;

// Meter styles
static lv_style_t style_meter_bg;

// Text styles
static lv_style_t style_text_primary;
static lv_style_t style_text_secondary;
static lv_style_t style_text_muted;
static lv_style_t style_text_hero;

// Button styles
static lv_style_t style_button_primary;
static lv_style_t style_button_touch;

// Effect card styles
static lv_style_t style_effect_card;
static lv_style_t style_effect_card_active;

// Font mapping - using only fonts available in lv_conf.h
static const lv_font_t* font_hero = &lv_font_montserrat_24;
static const lv_font_t* font_emphasis = &lv_font_montserrat_18;
static const lv_font_t* font_body = &lv_font_montserrat_16;
static const lv_font_t* font_small = &lv_font_montserrat_14;
static const lv_font_t* font_tiny = &lv_font_montserrat_12;

void ui_theme_init(void) {
    // === BACKGROUND STYLES ===
    
    // Main background - deepest dark
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, COLOR_BG);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
    lv_style_set_pad_all(&style_bg, 0);
    
    // Surface - cards and containers
    lv_style_init(&style_surface);
    lv_style_set_bg_color(&style_surface, COLOR_SURFACE);
    lv_style_set_bg_opa(&style_surface, LV_OPA_COVER);
    lv_style_set_radius(&style_surface, THEME_RADIUS_M);
    lv_style_set_pad_all(&style_surface, THEME_SPACING_M);
    
    // Elevated surface - headers, modals
    lv_style_init(&style_surface_elevated);
    lv_style_set_bg_color(&style_surface_elevated, COLOR_SURFACE_ELEVATED);
    lv_style_set_bg_opa(&style_surface_elevated, LV_OPA_COVER);
    lv_style_set_radius(&style_surface_elevated, 0);
    lv_style_set_pad_hor(&style_surface_elevated, THEME_SPACING_M);
    lv_style_set_pad_ver(&style_surface_elevated, THEME_SPACING_S);
    
    // === CARD STYLES ===
    
    // Standard card
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_SURFACE);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, THEME_RADIUS_L);
    lv_style_set_pad_all(&style_card, THEME_SPACING_M);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, COLOR_BORDER_SUBTLE);
    
    // Active card (selected/focused)
    lv_style_init(&style_card_active);
    lv_style_set_bg_color(&style_card_active, COLOR_SURFACE_SELECTED);
    lv_style_set_bg_opa(&style_card_active, LV_OPA_COVER);
    lv_style_set_radius(&style_card_active, THEME_RADIUS_L);
    lv_style_set_pad_all(&style_card_active, THEME_SPACING_M);
    lv_style_set_border_width(&style_card_active, 2);
    lv_style_set_border_color(&style_card_active, COLOR_ACCENT_PRIMARY);
    
    // === HEADER STYLE ===
    
    lv_style_init(&style_header);
    lv_style_set_bg_color(&style_header, COLOR_SURFACE_ELEVATED);
    lv_style_set_bg_opa(&style_header, LV_OPA_COVER);
    lv_style_set_radius(&style_header, 0);
    lv_style_set_pad_hor(&style_header, THEME_SPACING_M);
    lv_style_set_pad_ver(&style_header, THEME_SPACING_S);
    lv_style_set_border_width(&style_header, 0);
    
    // === NAVBAR STYLE ===
    
    lv_style_init(&style_navbar);
    lv_style_set_bg_color(&style_navbar, COLOR_SURFACE_ELEVATED);
    lv_style_set_bg_opa(&style_navbar, LV_OPA_COVER);
    lv_style_set_radius(&style_navbar, 0);
    lv_style_set_pad_all(&style_navbar, THEME_SPACING_XS);
    lv_style_set_border_width(&style_navbar, 0);
    
    // === METER STYLE ===
    
    lv_style_init(&style_meter_bg);
    lv_style_set_bg_color(&style_meter_bg, COLOR_DISABLED);
    lv_style_set_bg_opa(&style_meter_bg, LV_OPA_COVER);
    lv_style_set_radius(&style_meter_bg, THEME_RADIUS_S);
    
    // === TEXT STYLES ===
    
    // Primary text - main content
    lv_style_init(&style_text_primary);
    lv_style_set_text_color(&style_text_primary, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_text_primary, font_body);
    
    // Secondary text - labels
    lv_style_init(&style_text_secondary);
    lv_style_set_text_color(&style_text_secondary, COLOR_TEXT_SECONDARY);
    lv_style_set_text_font(&style_text_secondary, font_small);
    
    // Muted text - disabled/inactive
    lv_style_init(&style_text_muted);
    lv_style_set_text_color(&style_text_muted, COLOR_TEXT_MUTED);
    lv_style_set_text_font(&style_text_muted, font_small);
    
    // Hero text - musical data (notes, big values)
    lv_style_init(&style_text_hero);
    lv_style_set_text_color(&style_text_hero, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_text_hero, font_hero);
    
    // === BUTTON STYLES ===
    
    // Primary button - accent color
    lv_style_init(&style_button_primary);
    lv_style_set_bg_color(&style_button_primary, COLOR_ACCENT_PRIMARY);
    lv_style_set_bg_opa(&style_button_primary, LV_OPA_COVER);
    lv_style_set_radius(&style_button_primary, THEME_RADIUS_M);
    lv_style_set_pad_hor(&style_button_primary, THEME_SPACING_L);
    lv_style_set_pad_ver(&style_button_primary, THEME_SPACING_M);
    lv_style_set_text_color(&style_button_primary, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_button_primary, font_body);
    
    // Touch button - large target for stage use
    lv_style_init(&style_button_touch);
    lv_style_set_bg_color(&style_button_touch, COLOR_SURFACE);
    lv_style_set_bg_opa(&style_button_touch, LV_OPA_COVER);
    lv_style_set_radius(&style_button_touch, THEME_RADIUS_M);
    lv_style_set_pad_all(&style_button_touch, THEME_SPACING_M);
    lv_style_set_border_width(&style_button_touch, 1);
    lv_style_set_border_color(&style_button_touch, COLOR_BORDER_SUBTLE);
    lv_style_set_text_color(&style_button_touch, COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_button_touch, font_body);
    
    // === EFFECT CARD STYLES ===
    
    // Default effect card - subtle, not green
    lv_style_init(&style_effect_card);
    lv_style_set_bg_color(&style_effect_card, COLOR_EFFECT_INACTIVE);
    lv_style_set_bg_opa(&style_effect_card, LV_OPA_COVER);
    lv_style_set_radius(&style_effect_card, THEME_RADIUS_M);
    lv_style_set_pad_all(&style_effect_card, THEME_SPACING_S);
    lv_style_set_border_width(&style_effect_card, 1);
    lv_style_set_border_color(&style_effect_card, COLOR_BORDER_SUBTLE);
    
    // Active effect card - accent border, not full green
    lv_style_init(&style_effect_card_active);
    lv_style_set_bg_color(&style_effect_card_active, COLOR_EFFECT_ACTIVE);
    lv_style_set_bg_opa(&style_effect_card_active, LV_OPA_COVER);
    lv_style_set_radius(&style_effect_card_active, THEME_RADIUS_M);
    lv_style_set_pad_all(&style_effect_card_active, THEME_SPACING_S);
    lv_style_set_border_width(&style_effect_card_active, 2);
    lv_style_set_border_color(&style_effect_card_active, COLOR_ACCENT_PRIMARY);
}

lv_color_t UiTheme_get_color(UiThemeColor color) {
    switch (color) {
        case THEME_COLOR_BG: return COLOR_BG;
        case THEME_COLOR_SURFACE: return COLOR_SURFACE;
        case THEME_COLOR_SURFACE_ELEVATED: return COLOR_SURFACE_ELEVATED;
        case THEME_COLOR_SURFACE_SELECTED: return COLOR_SURFACE_SELECTED;
        case THEME_COLOR_BORDER_SUBTLE: return COLOR_BORDER_SUBTLE;
        case THEME_COLOR_BORDER_ACTIVE: return COLOR_BORDER_ACTIVE;
        case THEME_COLOR_TEXT_PRIMARY: return COLOR_TEXT_PRIMARY;
        case THEME_COLOR_TEXT_SECONDARY: return COLOR_TEXT_SECONDARY;
        case THEME_COLOR_TEXT_MUTED: return COLOR_TEXT_MUTED;
        case THEME_COLOR_ACCENT_PRIMARY: return COLOR_ACCENT_PRIMARY;
        case THEME_COLOR_ACCENT_SECONDARY: return COLOR_ACCENT_SECONDARY;
        case THEME_COLOR_ACCENT_BRIGHT: return COLOR_ACCENT_BRIGHT;
        case THEME_COLOR_SUCCESS: return COLOR_SUCCESS;
        case THEME_COLOR_WARNING: return COLOR_WARNING;
        case THEME_COLOR_ERROR: return COLOR_ERROR;
        case THEME_COLOR_LINK_OK: return COLOR_LINK_OK;
        case THEME_COLOR_LINK_LOST: return COLOR_LINK_LOST;
        case THEME_COLOR_METER_SAFE: return COLOR_METER_SAFE;
        case THEME_COLOR_METER_WARNING: return COLOR_METER_WARNING;
        case THEME_COLOR_METER_CLIP: return COLOR_METER_CLIP;
        case THEME_COLOR_EFFECT_ACTIVE: return COLOR_EFFECT_ACTIVE;
        case THEME_COLOR_EFFECT_INACTIVE: return COLOR_EFFECT_INACTIVE;
        case THEME_COLOR_EFFECT_PRESSED: return COLOR_EFFECT_PRESSED;
        case THEME_COLOR_FS_PRESSED: return COLOR_FS_PRESSED;
        case THEME_COLOR_DISABLED: return COLOR_DISABLED;
        default: return COLOR_BG;
    }
}

const lv_font_t* ui_theme_get_font(UiThemeFont font) {
    switch (font) {
        case THEME_FONT_HERO: return font_hero;
        case THEME_FONT_EMPHASIS: return font_emphasis;
        case THEME_FONT_BODY: return font_body;
        case THEME_FONT_SMALL: return font_small;
        case THEME_FONT_TINY: return font_tiny;
        default: return font_body;
    }
}

lv_style_t* ui_style_get_bg(void) {
    return &style_bg;
}

lv_style_t* ui_style_get_surface(void) {
    return &style_surface;
}

lv_style_t* ui_style_get_surface_elevated(void) {
    return &style_surface_elevated;
}

lv_style_t* ui_style_get_card(void) {
    return &style_card;
}

lv_style_t* ui_style_get_card_active(void) {
    return &style_card_active;
}

lv_style_t* ui_style_get_header(void) {
    return &style_header;
}

lv_style_t* ui_style_get_navbar(void) {
    return &style_navbar;
}

lv_style_t* ui_style_get_meter_bg(void) {
    return &style_meter_bg;
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

lv_style_t* ui_style_get_text_hero(void) {
    return &style_text_hero;
}

lv_style_t* ui_style_get_button_primary(void) {
    return &style_button_primary;
}

lv_style_t* ui_style_get_button_touch(void) {
    return &style_button_touch;
}

lv_style_t* ui_style_get_effect_card(void) {
    return &style_effect_card;
}

lv_style_t* ui_style_get_effect_card_active(void) {
    return &style_effect_card_active;
}
