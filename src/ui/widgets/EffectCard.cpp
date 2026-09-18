/**
 * VoxP4 CYD - Effect Card Widget Implementation
 */

#include "EffectCard.h"
#include "../UiTheme.h"

static const char* effect_names[] = {
    "HARMONY",
    "REVERB",
    "DELAY",
    "LIMITER"
};

EffectCard_t* effect_card_create(lv_obj_t* parent, int32_t x, int32_t y, 
                                  EffectType_t effect_type) {
    EffectCard_t* card = (EffectCard_t*)lv_mem_alloc(sizeof(EffectCard_t));
    if (!card) return NULL;
    
    card->effect_type = effect_type;
    card->is_enabled = false;
    
    card->card = lv_obj_create(parent);
    lv_obj_set_size(card->card, 72, 46);
    lv_obj_set_pos(card->card, x, y);
    lv_obj_set_style_bg_color(card->card, COLOR_SURFACE, 0);
    lv_obj_set_style_border_color(card->card, COLOR_SEPARATOR, 0);
    lv_obj_set_style_border_width(card->card, 1, 0);
    lv_obj_set_style_pad_all(card->card, SPACING_XS, 0);
    lv_obj_set_style_radius(card->card, RADIUS_M, 0);
    
    card->name_label = lv_label_create(card->card);
    lv_label_set_text(card->name_label, effect_names[effect_type]);
    lv_obj_align(card->name_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(card->name_label, FONT_TINY, 0);
    lv_obj_set_style_text_color(card->name_label, COLOR_TEXT_SECONDARY, 0);
    
    card->status_indicator = lv_obj_create(card->card);
    lv_obj_set_size(card->status_indicator, 7, 7);
    lv_obj_align(card->status_indicator, LV_ALIGN_TOP_RIGHT, 0, 1);
    lv_obj_set_style_bg_color(card->status_indicator, COLOR_DISABLED, 0);
    lv_obj_set_style_border_width(card->status_indicator, 0, 0);
    lv_obj_set_style_radius(card->status_indicator, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(card->status_indicator, 0, 0);
    
    card->param_label = lv_label_create(card->card);
    lv_label_set_text(card->param_label, "--");
    lv_obj_align(card->param_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_font(card->param_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(card->param_label, COLOR_TEXT_MUTED, 0);
    
    return card;
}

void effect_card_set_enabled(EffectCard_t* card, bool enabled) {
    if (!card) return;
    
    card->is_enabled = enabled;
    
    // Orange LED + accent border when active; a small highlight, never a filled card.
    lv_obj_set_style_bg_color(card->status_indicator, 
        enabled ? COLOR_ACCENT : COLOR_DISABLED, 0);
    lv_obj_set_style_border_color(card->card, 
        enabled ? COLOR_ACCENT : COLOR_SEPARATOR, 0);
    lv_obj_set_style_text_color(card->name_label,
        enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_color(card->param_label,
        enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
}

void effect_card_set_param(EffectCard_t* card, const char* param_text) {
    if (!card) return;
    lv_label_set_text(card->param_label, param_text);
}

EffectType_t effect_card_get_type(EffectCard_t* card) {
    if (!card) return EFFECT_HARMONY;
    return card->effect_type;
}

bool effect_card_is_enabled(EffectCard_t* card) {
    if (!card) return false;
    return card->is_enabled;
}
