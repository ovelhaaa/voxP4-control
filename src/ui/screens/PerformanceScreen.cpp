#include "PerformanceScreen.h"
#include "../UiTheme.h"
#include "../widgets/VuMeter.h"
#include "../widgets/EffectCard.h"
#include <cstdio>

static lv_obj_t* performance_container = nullptr;
static lv_obj_t* header_preset_label = nullptr;
static lv_obj_t* header_link_label = nullptr;
static VuMeter_t* input_meter = nullptr;
static VuMeter_t* output_meter = nullptr;
static lv_obj_t* pitch_note_label = nullptr;
static lv_obj_t* pitch_freq_label = nullptr;
static lv_obj_t* voiced_label = nullptr;
static EffectCard_t* effect_cards[4] = {nullptr};
static lv_obj_t* fs_container = nullptr;
static lv_obj_t* fs1_label = nullptr;
static lv_obj_t* fs2_label = nullptr;
static lv_obj_t* fs1_indicator = nullptr;
static lv_obj_t* fs2_indicator = nullptr;

// Effect names and default params
static const char* effect_names[] = {"HARMONY", "REVERB", "LIMIT", "DELAY"};
static const char* effect_default_params[] = {"+3rd", "18%", "-3dB", "OFF"};

void performance_screen_init(lv_obj_t* parent) {
    // Clear parent first
    lv_obj_clean(parent);
    
    // Create main container with theme bg style
    performance_container = lv_obj_create(parent);
    lv_obj_set_size(performance_container, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(performance_container, ui_style_get_bg(), 0);
    lv_obj_set_style_pad_all(performance_container, THEME_SPACING_XS, 0);
    lv_obj_set_style_border_width(performance_container, 0, 0);
    lv_obj_set_flex_flow(performance_container, LV_FLEX_FLOW_COLUMN);
    
    // === HEADER (28px height) ===
    lv_obj_t* header = lv_obj_create(performance_container);
    lv_obj_set_size(header, LV_PCT(100), THEME_HEADER_HEIGHT);
    lv_obj_add_style(header, ui_style_get_header(), 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Preset name
    header_preset_label = lv_label_create(header);
    lv_label_set_text(header_preset_label, "P03 Lead Air");
    lv_obj_set_style_text_color(header_preset_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(header_preset_label, ui_theme_get_font(THEME_FONT_BODY), 0);
    
    // Link indicator
    header_link_label = lv_label_create(header);
    lv_label_set_text(header_link_label, "LINK LOST");
    lv_obj_set_style_text_color(header_link_label, COLOR_LINK_LOST, 0);
    lv_obj_set_style_text_font(header_link_label, ui_theme_get_font(THEME_FONT_SMALL), 0);
    
    // === METERS SECTION (60px) ===
    lv_obj_t* meters_area = lv_obj_create(performance_container);
    lv_obj_set_size(meters_area, LV_PCT(100), 60);
    lv_obj_set_style_bg_color(meters_area, COLOR_BG, 0);
    lv_obj_set_style_border_width(meters_area, 0, 0);
    lv_obj_set_style_pad_row(meters_area, THEME_SPACING_S, 0);
    lv_obj_set_style_pad_hor(meters_area, THEME_SPACING_M, 0);
    lv_obj_set_flex_flow(meters_area, LV_FLEX_FLOW_COLUMN);
    
    // Input meter usando VuMeter widget horizontal
    input_meter = vu_meter_create(meters_area, 0, 0, lv_obj_get_width(meters_area), 24, "IN");
    
    // Output meter
    output_meter = vu_meter_create(meters_area, 0, 0, lv_obj_get_width(meters_area), 24, "OUT");
    
    // === PITCH SECTION (48px) - Musical hierarchy ===
    lv_obj_t* pitch_area = lv_obj_create(performance_container);
    lv_obj_set_size(pitch_area, LV_PCT(100), 48);
    lv_obj_add_style(pitch_area, ui_style_get_card(), 0);
    lv_obj_set_flex_flow(pitch_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pitch_area, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(pitch_area, THEME_SPACING_L, 0);
    
    // Note name (hero)
    pitch_note_label = lv_label_create(pitch_area);
    lv_label_set_text(pitch_note_label, "A3");
    lv_obj_set_style_text_color(pitch_note_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(pitch_note_label, ui_theme_get_font(THEME_FONT_HERO), 0);
    
    // Frequency (emphasis)
    pitch_freq_label = lv_label_create(pitch_area);
    lv_label_set_text(pitch_freq_label, "220.1 Hz");
    lv_obj_set_style_text_color(pitch_freq_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(pitch_freq_label, ui_theme_get_font(THEME_FONT_EMPHASIS), 0);
    
    // Voiced status
    voiced_label = lv_label_create(pitch_area);
    lv_label_set_text(voiced_label, "VOICED");
    lv_obj_set_style_text_color(voiced_label, COLOR_SUCCESS, 0);
    lv_obj_set_style_text_font(voiced_label, ui_theme_get_font(THEME_FONT_SMALL), 0);
    
    // === EFFECT CARDS (56px) - Using EffectCard widgets ===
    lv_obj_t* effects_area = lv_obj_create(performance_container);
    lv_obj_set_size(effects_area, LV_PCT(100), 56);
    lv_obj_set_style_bg_color(effects_area, COLOR_BG, 0);
    lv_obj_set_style_border_width(effects_area, 0, 0);
    lv_obj_set_style_pad_column(effects_area, THEME_SPACING_S, 0);
    lv_obj_set_flex_flow(effects_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(effects_area, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    for (int i = 0; i < 4; i++) {
        effect_cards[i] = effect_card_create(effects_area, 0, 0, (EffectType_t)i);
        effect_card_set_param(effect_cards[i], effect_default_params[i]);
        effect_card_set_enabled(effect_cards[i], false);
        
        // Add click event to toggle effect
        lv_obj_add_event_cb(effect_cards[i]->card, [](lv_event_t* e) {
            EffectCard_t* card = (EffectCard_t*)lv_event_get_user_data(e);
            if (card) {
                bool currently_on = effect_card_is_enabled(card);
                effect_card_set_enabled(card, !currently_on);
                // TODO: Send command to P4
            }
        }, LV_EVENT_CLICKED, effect_cards[i]);
    }
    
    // === FOOTSWITCH BAR (36px) ===
    fs_container = lv_obj_create(performance_container);
    lv_obj_set_size(fs_container, LV_PCT(100), 36);
    lv_obj_add_style(fs_container, ui_style_get_surface_elevated(), 0);
    lv_obj_set_flex_flow(fs_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(fs_container, THEME_SPACING_M, 0);
    
    // FS1
    lv_obj_t* fs1_area = lv_obj_create(fs_container);
    lv_obj_set_size(fs1_area, LV_PCT(48), 28);
    lv_obj_set_style_bg_color(fs1_area, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(fs1_area, THEME_RADIUS_M, 0);
    lv_obj_set_style_border_width(fs1_area, 0, 0);
    lv_obj_set_flex_flow(fs1_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs1_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(fs1_area, THEME_SPACING_S, 0);
    
    fs1_indicator = lv_obj_create(fs1_area);
    lv_obj_set_size(fs1_indicator, 8, 8);
    lv_obj_set_style_bg_color(fs1_indicator, COLOR_DISABLED, 0);
    lv_obj_set_style_radius(fs1_indicator, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(fs1_indicator, 0, 0);
    
    fs1_label = lv_label_create(fs1_area);
    lv_label_set_text(fs1_label, "FS1 HARMONY");
    lv_obj_set_style_text_color(fs1_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(fs1_label, ui_theme_get_font(THEME_FONT_SMALL), 0);
    
    // FS2
    lv_obj_t* fs2_area = lv_obj_create(fs_container);
    lv_obj_set_size(fs2_area, LV_PCT(48), 28);
    lv_obj_set_style_bg_color(fs2_area, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(fs2_area, THEME_RADIUS_M, 0);
    lv_obj_set_style_border_width(fs2_area, 0, 0);
    lv_obj_set_flex_flow(fs2_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs2_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(fs2_area, THEME_SPACING_S, 0);
    
    fs2_indicator = lv_obj_create(fs2_area);
    lv_obj_set_size(fs2_indicator, 8, 8);
    lv_obj_set_style_bg_color(fs2_indicator, COLOR_DISABLED, 0);
    lv_obj_set_style_radius(fs2_indicator, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(fs2_indicator, 0, 0);
    
    fs2_label = lv_label_create(fs2_area);
    lv_label_set_text(fs2_label, "FS2 REVERB");
    lv_obj_set_style_text_color(fs2_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(fs2_label, ui_theme_get_font(THEME_FONT_SMALL), 0);
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
        lv_obj_set_style_text_color(voiced_label, voiced ? COLOR_SUCCESS : COLOR_TEXT_MUTED, 0);
    }
}

void performance_update_effect(int effectIndex, bool enabled) {
    if (effectIndex >= 0 && effectIndex < 4 && effect_cards[effectIndex]) {
        effect_card_set_enabled(effect_cards[effectIndex], enabled);
    }
}

void performance_update_preset(const char* name) {
    if (header_preset_label) {
        lv_label_set_text(header_preset_label, name);
    }
}

void performance_update_link(bool connected) {
    if (header_link_label) {
        lv_label_set_text(header_link_label, connected ? "LINK OK" : "LINK LOST");
        lv_obj_set_style_text_color(header_link_label, 
            connected ? COLOR_LINK_OK : COLOR_LINK_LOST, 0);
    }
}

void performance_update_footswitch(int fsIndex, const char* label, bool pressed) {
    lv_obj_t* fs_label = (fsIndex == 0) ? fs1_label : fs2_label;
    lv_obj_t* fs_indicator = (fsIndex == 0) ? fs1_indicator : fs2_indicator;
    
    if (fs_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "FS%d %s", fsIndex + 1, label);
        lv_label_set_text(fs_label, buf);
    }
    if (fs_indicator) {
        lv_obj_set_style_bg_color(fs_indicator, 
            pressed ? COLOR_FS_PRESSED : COLOR_DISABLED, 0);
    }
}
