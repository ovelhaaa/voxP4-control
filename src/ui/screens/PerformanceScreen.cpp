#include "PerformanceScreen.h"
#include "../UiTheme.h"
#include "../widgets/VuMeter.h"
#include "../widgets/EffectCard.h"
#include <cstdio>

static lv_obj_t* performance_container = nullptr;
static lv_obj_t* preset_label = nullptr;
static lv_obj_t* link_indicator = nullptr;
static VuMeter_t* input_meter = nullptr;
static VuMeter_t* output_meter = nullptr;
static lv_obj_t* pitch_note_label = nullptr;
static lv_obj_t* pitch_freq_label = nullptr;
static lv_obj_t* voiced_label = nullptr;
static EffectCard_t* effect_cards[4] = {nullptr};
static lv_obj_t* fs_containers[2] = {nullptr};
static lv_obj_t* fs_labels[2] = {nullptr};

// Effect names matching EffectType_t enum order
static const char* effect_names[] = {"HARMONY", "REVERB", "DELAY", "LIMIT"};

// Default params para cada efeito (mesma ordem do enum)
static const char* effect_default_params[] = {"+3rd", "18%", "--", "--"};

void performance_screen_init(lv_obj_t* parent) {
    // Create main container with padding
    performance_container = lv_obj_create(parent);
    lv_obj_set_size(performance_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(performance_container, COLOR_BG_DARK, 0);
    lv_obj_set_style_pad_all(performance_container, SPACING_XS, 0);
    lv_obj_set_style_border_width(performance_container, 0, 0);
    lv_obj_set_flex_flow(performance_container, LV_FLEX_FLOW_COLUMN);
    
    // === HEADER (28px) ===
    lv_obj_t* header = lv_obj_create(performance_container);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, COLOR_BG_HEADER, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Preset name label
    preset_label = lv_label_create(header);
    lv_label_set_text(preset_label, "P03 Lead Air");
    lv_obj_set_style_text_color(preset_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(preset_label, FONT_EMPHASIS, 0);
    
    // Link indicator
    link_indicator = lv_label_create(header);
    lv_label_set_text(link_indicator, "○ LINK");
    lv_obj_set_style_text_color(link_indicator, COLOR_LINK_LOST, 0);
    lv_obj_set_style_text_font(link_indicator, FONT_BODY, 0);
    
    // === METERS SECTION (52px) - Horizontal meters ===
    lv_obj_t* meters_container = lv_obj_create(performance_container);
    lv_obj_set_size(meters_container, LV_PCT(100), 52);
    lv_obj_set_style_bg_color(meters_container, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(meters_container, 0, 0);
    lv_obj_set_style_pad_row(meters_container, SPACING_S, 0);
    lv_obj_set_flex_flow(meters_container, LV_FLEX_FLOW_COLUMN);
    
    // Input meter row - horizontal bar com label
    lv_obj_t* input_row = lv_obj_create(meters_container);
    lv_obj_set_size(input_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(input_row, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(input_row, 0, 0);
    lv_obj_set_style_pad_column(input_row, SPACING_S, 0);
    lv_obj_set_flex_flow(input_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(input_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* in_label = lv_label_create(input_row);
    lv_label_set_text(in_label, "IN");
    lv_obj_set_style_text_color(in_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(in_label, FONT_SMALL, 0);
    
    // Usar VuMeter widget horizontal (implementar como bar customizada)
    input_meter = vu_meter_create(input_row, 0, 0, 16, "");
    if (input_meter) {
        lv_obj_set_size(vu_meter_get_container(input_meter), 180, 16);
    }
    
    input_row = lv_obj_create(meters_container);
    lv_obj_set_size(input_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(input_row, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(input_row, 0, 0);
    lv_obj_set_style_pad_column(input_row, SPACING_S, 0);
    lv_obj_set_flex_flow(input_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(input_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* out_label = lv_label_create(input_row);
    lv_label_set_text(out_label, "OUT");
    lv_obj_set_style_text_color(out_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(out_label, FONT_SMALL, 0);
    
    output_meter = vu_meter_create(input_row, 0, 0, 16, "");
    if (output_meter) {
        lv_obj_set_size(vu_meter_get_container(output_meter), 180, 16);
    }
    
    // === PITCH SECTION (40px) ===
    lv_obj_t* pitch_container = lv_obj_create(performance_container);
    lv_obj_set_size(pitch_container, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(pitch_container, COLOR_BG_ELEVATED, 0);
    lv_obj_set_style_radius(pitch_container, RADIUS_M, 0);
    lv_obj_set_style_border_width(pitch_container, 1, 0);
    lv_obj_set_style_border_color(pitch_container, COLOR_BORDER_SUBTLE, 0);
    lv_obj_set_style_pad_all(pitch_container, SPACING_M, 0);
    lv_obj_set_flex_flow(pitch_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pitch_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Nota musical em destaque
    pitch_note_label = lv_label_create(pitch_container);
    lv_label_set_text(pitch_note_label, "A3");
    lv_obj_set_style_text_color(pitch_note_label, COLOR_ACCENT_BRIGHT, 0);
    lv_obj_set_style_text_font(pitch_note_label, FONT_EMPHASIS, 0);
    
    // Frequência e status
    lv_obj_t* pitch_info = lv_obj_create(pitch_container);
    lv_obj_set_size(pitch_info, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(pitch_info, COLOR_BG_ELEVATED, 0);
    lv_obj_set_style_border_width(pitch_info, 0, 0);
    lv_obj_set_flex_flow(pitch_info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(pitch_info, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    pitch_freq_label = lv_label_create(pitch_info);
    lv_label_set_text(pitch_freq_label, "220.1 Hz");
    lv_obj_set_style_text_color(pitch_freq_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(pitch_freq_label, FONT_SMALL, 0);
    
    voiced_label = lv_label_create(pitch_info);
    lv_label_set_text(voiced_label, "UNVOICED");
    lv_obj_set_style_text_color(voiced_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(voiced_label, FONT_TINY, 0);
    
    // === EFFECT CARDS (56px) ===
    lv_obj_t* effects_container = lv_obj_create(performance_container);
    lv_obj_set_size(effects_container, LV_PCT(100), 56);
    lv_obj_set_style_bg_color(effects_container, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(effects_container, 0, 0);
    lv_obj_set_style_pad_column(effects_container, SPACING_S, 0);
    lv_obj_set_flex_flow(effects_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(effects_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Criar EffectCards usando a ordem correta do enum
    for (int i = 0; i < 4; i++) {
        // Ordem: HARMONY(0), REVERB(1), DELAY(2), LIMITER(3)
        EffectType_t type = (EffectType_t)i;
        effect_cards[i] = effect_card_create(effects_container, 0, 0, type);
        
        if (effect_cards[i]) {
            // Set default param
            effect_card_set_param(effect_cards[i], effect_default_params[i]);
            effect_card_set_enabled(effect_cards[i], false);
            
            // Registrar callback para toggle on tap
            lv_obj_add_flag(effect_cards[i]->card, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(effect_cards[i]->card, [](lv_event_t* e) {
                EffectCard_t* card = (EffectCard_t*)lv_event_get_user_data(e);
                if (card) {
                    bool new_state = !effect_card_is_enabled(card);
                    effect_card_set_enabled(card, new_state);
                    // Callback para UiApp será implementado
                }
            }, LV_EVENT_CLICKED, effect_cards[i]);
        }
    }
    
    // === FOOTSWITCH BAR (36px) ===
    lv_obj_t* fs_container = lv_obj_create(performance_container);
    lv_obj_set_size(fs_container, LV_PCT(100), 36);
    lv_obj_set_style_bg_color(fs_container, COLOR_BG_HEADER, 0);
    lv_obj_set_style_radius(fs_container, 0, 0);
    lv_obj_set_style_border_width(fs_container, 0, 0);
    lv_obj_set_style_pad_all(fs_container, SPACING_XS, 0);
    lv_obj_set_flex_flow(fs_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // FS1 e FS2 cards
    for (int i = 0; i < 2; i++) {
        fs_containers[i] = lv_obj_create(fs_container);
        // Calcular largura em pixels baseado no container pai (320px - padding)
        // 320 - 8(padding) = 312px disponíveis, dividido por 2 cards menos gap de 6px
        // Cada card: (312 - 6) / 2 = 153px ≈ LV_PCT(48)
        lv_obj_set_size(fs_containers[i], LV_PCT(48), LV_PCT(100));
        lv_obj_set_style_bg_color(fs_containers[i], COLOR_BG_SURFACE, 0);
        lv_obj_set_style_radius(fs_containers[i], RADIUS_S, 0);
        lv_obj_set_style_border_width(fs_containers[i], 0, 0);
        lv_obj_set_style_pad_all(fs_containers[i], SPACING_XS, 0);
        lv_obj_set_flex_flow(fs_containers[i], LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(fs_containers[i], LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        // Label FS#
        char fs_num[8];
        snprintf(fs_num, sizeof(fs_num), "FS%d", i + 1);
        lv_obj_t* num_label = lv_label_create(fs_containers[i]);
        lv_label_set_text(num_label, fs_num);
        lv_obj_set_style_text_color(num_label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(num_label, FONT_TINY, 0);
        
        // Label da ação
        fs_labels[i] = lv_label_create(fs_containers[i]);
        lv_label_set_text(fs_labels[i], i == 0 ? "HARMONY" : "REVERB");
        lv_obj_set_style_text_color(fs_labels[i], COLOR_TEXT_SECONDARY, 0);
        lv_obj_set_style_text_font(fs_labels[i], FONT_SMALL, 0);
    }
}

void performance_update_meters(float inputDb, float outputDb) {
    if (input_meter) vu_meter_update(input_meter, inputDb);
    if (output_meter) vu_meter_update(output_meter, outputDb);
}

void performance_update_pitch(float freqHz, const char* noteName, bool voiced) {
    if (pitch_note_label) {
        lv_label_set_text(pitch_note_label, noteName);
    }
    if (pitch_freq_label) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f Hz", freqHz);
        lv_label_set_text(pitch_freq_label, buf);
    }
    if (voiced_label) {
        lv_label_set_text(voiced_label, voiced ? "VOICED" : "UNVOICED");
        lv_obj_set_style_text_color(voiced_label, voiced ? COLOR_TEXT_SECONDARY : COLOR_TEXT_MUTED, 0);
    }
}

void performance_update_effect(int effectIndex, bool enabled) {
    // Validar índice antes de acessar
    if (effectIndex < 0 || effectIndex >= 4) return;
    
    if (effect_cards[effectIndex]) {
        effect_card_set_enabled(effect_cards[effectIndex], enabled);
    }
}

void performance_update_preset(const char* name) {
    if (preset_label) lv_label_set_text(preset_label, name);
}

void performance_update_link(bool connected) {
    if (link_indicator) {
        lv_label_set_text(link_indicator, connected ? "● LINK" : "○ LINK");
        lv_obj_set_style_text_color(link_indicator, 
            connected ? COLOR_LINK_OK : COLOR_LINK_LOST, 0);
    }
}

void performance_update_footswitch(int fsIndex, const char* label, bool pressed) {
    // Validar índice antes de acessar
    if (fsIndex < 0 || fsIndex > 1) return;
    
    if (fs_labels[fsIndex]) {
        lv_label_set_text(fs_labels[fsIndex], label);
        // Highlight visual quando pressionado - amber no texto
        lv_obj_set_style_text_color(fs_labels[fsIndex],
            pressed ? COLOR_FS_PRESSED : COLOR_TEXT_SECONDARY, 0);
        
        // Highlight no container do footswitch
        if (fs_containers[fsIndex]) {
            lv_obj_set_style_border_width(fs_containers[fsIndex], pressed ? 2 : 0, 0);
            lv_obj_set_style_border_color(fs_containers[fsIndex], 
                pressed ? COLOR_FS_PRESSED : COLOR_BORDER_SUBTLE, 0);
        }
    }
}
