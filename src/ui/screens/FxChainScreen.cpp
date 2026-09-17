#include "FxChainScreen.h"
#include "../UiTheme.h"

static lv_obj_t* fx_container = nullptr;
static lv_obj_t* effect_cards[4] = {nullptr, nullptr, nullptr, nullptr};

// Effect names matching EffectType_t enum order
static const char* effect_names[] = {"HARMONY", "REVERB", "DELAY", "LIMITER"};

void fx_chain_screen_init(lv_obj_t* parent) {
    // Create main container with padding
    fx_container = lv_obj_create(parent);
    lv_obj_set_size(fx_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(fx_container, COLOR_BG_DARK, 0);
    lv_obj_set_style_pad_all(fx_container, SPACING_M, 0);
    lv_obj_set_style_border_width(fx_container, 0, 0);
    lv_obj_set_flex_flow(fx_container, LV_FLEX_FLOW_COLUMN);
    
    // === HEADER ===
    lv_obj_t* header = lv_obj_create(fx_container);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, COLOR_BG_HEADER, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "FX CHAIN");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_EMPHASIS, 0);
    
    // === EFFECT CARDS ROW ===
    lv_obj_t* effects_row = lv_obj_create(fx_container);
    lv_obj_set_size(effects_row, LV_PCT(100), 72);
    lv_obj_set_style_bg_color(effects_row, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(effects_row, 0, 0);
    lv_obj_set_style_pad_column(effects_row, SPACING_S, 0);
    lv_obj_set_flex_flow(effects_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(effects_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Create 4 effect cards in chain order: HARMONY → REVERB → DELAY → LIMITER
    for (int i = 0; i < 4; i++) {
        lv_obj_t* card = lv_obj_create(effects_row);
        lv_obj_set_size(card, 72, 72);
        lv_obj_set_style_bg_color(card, COLOR_BG_SURFACE, 0);
        lv_obj_set_style_radius(card, RADIUS_M, 0);
        lv_obj_set_style_border_width(card, 2, 0);
        lv_obj_set_style_border_color(card, COLOR_BORDER_SUBTLE, 0);
        lv_obj_set_style_pad_all(card, SPACING_S, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        // Effect name
        lv_obj_t* name_label = lv_label_create(card);
        lv_label_set_text(name_label, effect_names[i]);
        lv_obj_set_style_text_color(name_label, COLOR_TEXT_SECONDARY, 0);
        lv_obj_set_style_text_font(name_label, FONT_SMALL, 0);
        
        // Status indicator (LED dot)
        lv_obj_t* led = lv_obj_create(card);
        lv_obj_set_size(led, 8, 8);
        lv_obj_set_style_bg_color(led, COLOR_EFFECT_LED_OFF, 0);
        lv_obj_set_style_radius(led, 4, 0);
        lv_obj_set_style_border_width(led, 0, 0);
        
        // Parameter value placeholder
        lv_obj_t* param_label = lv_label_create(card);
        lv_label_set_text(param_label, "--");
        lv_obj_set_style_text_color(param_label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(param_label, FONT_TINY, 0);
        
        effect_cards[i] = card;
    }
    
    // Arrow indicators between cards (visual flow)
    // Simplified: arrows shown via text labels in a separate row
    
    // === QUICK ACTIONS ===
    lv_obj_t* actions_row = lv_obj_create(fx_container);
    lv_obj_set_size(actions_row, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(actions_row, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(actions_row, 0, 0);
    lv_obj_set_style_pad_column(actions_row, SPACING_S, 0);
    lv_obj_set_flex_flow(actions_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actions_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // ALL ON button
    lv_obj_t* all_on_btn = lv_btn_create(actions_row);
    lv_obj_set_size(all_on_btn, 70, 32);
    lv_obj_set_style_bg_color(all_on_btn, COLOR_SUCCESS, 0);
    lv_obj_set_style_radius(all_on_btn, RADIUS_S, 0);
    lv_obj_t* all_on_label = lv_label_create(all_on_btn);
    lv_label_set_text(all_on_label, "ALL ON");
    lv_obj_center(all_on_label);
    lv_obj_set_style_text_color(all_on_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(all_on_label, FONT_SMALL, 0);
    
    // BYPASS ALL button
    lv_obj_t* bypass_btn = lv_btn_create(actions_row);
    lv_obj_set_size(bypass_btn, 70, 32);
    lv_obj_set_style_bg_color(bypass_btn, COLOR_WARNING, 0);
    lv_obj_set_style_radius(bypass_btn, RADIUS_S, 0);
    lv_obj_t* bypass_label = lv_label_create(bypass_btn);
    lv_label_set_text(bypass_label, "BYPASS");
    lv_obj_center(bypass_label);
    lv_obj_set_style_text_color(bypass_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(bypass_label, FONT_SMALL, 0);
}

void fx_chain_update_effect_state(int effectId, bool enabled, const char* paramName, const char* paramValue) {
    if (effectId < 0 || effectId >= 4 || !effect_cards[effectId]) return;
    
    lv_obj_t* card = effect_cards[effectId];
    
    // Update border color based on state
    lv_obj_set_style_border_color(card, enabled ? COLOR_BORDER_ACTIVE : COLOR_BORDER_SUBTLE, 0);
    
    // Find and update LED
    lv_obj_t* led = lv_obj_get_child(card, 1);
    if (led) {
        lv_obj_set_style_bg_color(led, enabled ? COLOR_EFFECT_LED_ON : COLOR_EFFECT_LED_OFF, 0);
    }
    
    // Update parameter value if provided
    if (paramValue) {
        lv_obj_t* param_label = lv_obj_get_child(card, 2);
        if (param_label) {
            lv_label_set_text(param_label, paramValue);
            lv_obj_set_style_text_color(param_label, enabled ? COLOR_ACCENT_CYAN : COLOR_TEXT_MUTED, 0);
        }
    }
}
