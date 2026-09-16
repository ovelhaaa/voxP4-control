/**
 * VoxP4 CYD - Effect Card Widget Implementation
 * 
 * Design: card sutil com border/LED de status, NÃO verde sólido
 * - Estado ativo: border accent + LED cyan
 * - Estado inativo: border subtle + LED gray
 * - Texto sempre legível
 */

#include "EffectCard.h"
#include "../UiTheme.h"

static const char* effect_names[] = {
    "HARMONY",
    "REVERB",
    "DELAY",
    "LIMIT"
};

EffectCard_t* effect_card_create(lv_obj_t* parent, int32_t x, int32_t y, 
                                  EffectType_t effect_type) {
    EffectCard_t* card = (EffectCard_t*)lv_mem_alloc(sizeof(EffectCard_t));
    if (!card) return NULL;
    lv_memset(card, 0, sizeof(EffectCard_t));
    
    card->effect_type = effect_type;
    card->is_enabled = false;
    
    // Card container
    card->card = lv_obj_create(parent);
    lv_obj_set_size(card->card, 70, 50);
    lv_obj_set_pos(card->card, x, y);
    lv_obj_set_style_bg_color(card->card, COLOR_EFFECT_INACTIVE, 0);
    lv_obj_set_style_border_color(card->card, COLOR_BORDER_SUBTLE, 0);
    lv_obj_set_style_border_width(card->card, 1, 0);
    lv_obj_set_style_pad_all(card->card, THEME_SPACING_S, 0);
    lv_obj_set_style_radius(card->card, THEME_RADIUS_M, 0);
    
    // Nome do efeito
    card->name_label = lv_label_create(card->card);
    lv_label_set_text(card->name_label, effect_names[effect_type]);
    lv_obj_align(card->name_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(card->name_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(card->name_label, COLOR_TEXT_PRIMARY, 0);
    
    // LED indicador de status (circular, pequeno)
    card->status_indicator = lv_obj_create(card->card);
    lv_obj_set_size(card->status_indicator, 6, 6);
    lv_obj_align(card->status_indicator, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(card->status_indicator, COLOR_DISABLED, 0);
    lv_obj_set_style_border_width(card->status_indicator, 0, 0);
    lv_obj_set_style_radius(card->status_indicator, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(card->status_indicator, 0, 0);
    
    // Parâmetro principal
    card->param_label = lv_label_create(card->card);
    lv_label_set_text(card->param_label, "--");
    lv_obj_align(card->param_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_font(card->param_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(card->param_label, COLOR_ACCENT_BRIGHT, 0);
    
    return card;
}

void effect_card_set_enabled(EffectCard_t* card, bool enabled) {
    if (!card) return;
    
    card->is_enabled = enabled;
    
    // Border e background mudam sutilmente, NÃO verde sólido
    if (enabled) {
        lv_obj_set_style_border_color(card->card, COLOR_ACCENT_PRIMARY, 0);
        lv_obj_set_style_border_width(card->card, 2, 0);
        lv_obj_set_style_bg_color(card->card, COLOR_EFFECT_ACTIVE, 0);
        // LED cyan quando ativo
        lv_obj_set_style_bg_color(card->status_indicator, COLOR_ACCENT_BRIGHT, 0);
    } else {
        lv_obj_set_style_border_color(card->card, COLOR_BORDER_SUBTLE, 0);
        lv_obj_set_style_border_width(card->card, 1, 0);
        lv_obj_set_style_bg_color(card->card, COLOR_EFFECT_INACTIVE, 0);
        // LED gray quando inativo
        lv_obj_set_style_bg_color(card->status_indicator, COLOR_DISABLED, 0);
    }
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
