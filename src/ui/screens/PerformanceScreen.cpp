#include "PerformanceScreen.h"
#include "../UiTheme.h"

static lv_obj_t* performance_container = nullptr;
static lv_obj_t* preset_label = nullptr;
static lv_obj_t* link_indicator = nullptr;
static lv_obj_t* input_meter = nullptr;
static lv_obj_t* output_meter = nullptr;
static lv_obj_t* pitch_label = nullptr;
static lv_obj_t* voiced_label = nullptr;
static lv_obj_t* effect_buttons[4] = {nullptr};
static lv_obj_t* fs_labels[2] = {nullptr};

// Effect names for buttons
static const char* effect_names[] = {"HARMONY", "REVERB", "LIMIT", "DELAY"};

void performance_screen_init(lv_obj_t* parent) {
    // Create main container with padding
    performance_container = lv_obj_create(parent);
    lv_obj_set_size(performance_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(performance_container, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_pad_all(performance_container, 8, 0);
    lv_obj_set_style_border_width(performance_container, 0, 0);
    lv_obj_set_flex_flow(performance_container, LV_FLEX_FLOW_COLUMN);
    
    // === HEADER ===
    lv_obj_t* header = lv_obj_create(performance_container);
    lv_obj_set_size(header, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(header, lv_color_make(0x2A, 0x2A, 0x3A), 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Preset name label
    preset_label = lv_label_create(header);
    lv_label_set_text(preset_label, "P03 Lead Air");
    lv_obj_set_style_text_color(preset_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(preset_label, &lv_font_montserrat_16, 0);
    
    // Link indicator
    link_indicator = lv_label_create(header);
    lv_label_set_text(link_indicator, "● LINK");
    lv_obj_set_style_text_color(link_indicator, lv_color_make(0xEF5350), 0);  // Red = not connected
    lv_obj_set_style_text_font(preset_label, &lv_font_montserrat_14, 0);
    
    // === METERS SECTION ===
    lv_obj_t* meters_container = lv_obj_create(performance_container);
    lv_obj_set_size(meters_container, LV_PCT(100), 60);
    lv_obj_set_style_bg_color(meters_container, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_border_width(meters_container, 0, 0);
    lv_obj_set_style_pad_row(meters_container, 8, 0);
    lv_obj_set_flex_flow(meters_container, LV_FLEX_FLOW_COLUMN);
    
    // Input meter row
    lv_obj_t* input_row = lv_obj_create(meters_container);
    lv_obj_set_size(input_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(input_row, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_border_width(input_row, 0, 0);
    lv_obj_set_style_pad_column(input_row, 8, 0);
    lv_obj_set_flex_flow(input_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(input_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* in_label = lv_label_create(input_row);
    lv_label_set_text(in_label, "IN");
    lv_obj_set_style_text_color(in_label, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(in_label, &lv_font_montserrat_12, 0);
    
    input_meter = lv_bar_create(input_row);
    lv_obj_set_size(input_meter, 180, 16);
    lv_bar_set_range(input_meter, -60, 0);
    lv_bar_set_value(input_meter, -60, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(input_meter, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_bg_color(lv_bar_get_indic(input_meter), lv_color_make(0x4CAF50), 0);
    
    lv_obj_t* in_db_label = lv_label_create(input_row);
    lv_label_set_text(in_db_label, "-60 dB");
    lv_obj_set_style_text_color(in_db_label, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(in_db_label, &lv_font_montserrat_12, 0);
    
    // Output meter row
    lv_obj_t* output_row = lv_obj_create(meters_container);
    lv_obj_set_size(output_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(output_row, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_border_width(output_row, 0, 0);
    lv_obj_set_style_pad_column(output_row, 8, 0);
    lv_obj_set_flex_flow(output_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(output_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* out_label = lv_label_create(output_row);
    lv_label_set_text(out_label, "OUT");
    lv_obj_set_style_text_color(out_label, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(out_label, &lv_font_montserrat_12, 0);
    
    output_meter = lv_bar_create(output_row);
    lv_obj_set_size(output_meter, 180, 16);
    lv_bar_set_range(output_meter, -60, 0);
    lv_bar_set_value(output_meter, -60, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(output_meter, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_bg_color(lv_bar_get_indic(output_meter), lv_color_make(0x4CAF50), 0);
    
    lv_obj_t* out_db_label = lv_label_create(output_row);
    lv_label_set_text(out_db_label, "-60 dB");
    lv_obj_set_style_text_color(out_db_label, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(out_db_label, &lv_font_montserrat_12, 0);
    
    // === PITCH SECTION ===
    lv_obj_t* pitch_container = lv_obj_create(performance_container);
    lv_obj_set_size(pitch_container, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(pitch_container, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_radius(pitch_container, 8, 0);
    lv_obj_set_style_border_width(pitch_container, 0, 0);
    lv_obj_set_style_pad_all(pitch_container, 8, 0);
    lv_obj_set_flex_flow(pitch_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pitch_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    pitch_label = lv_label_create(pitch_container);
    lv_label_set_text(pitch_label, "Pitch: A3  220.1Hz");
    lv_obj_set_style_text_color(pitch_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(pitch_label, &lv_font_montserrat_14, 0);
    
    voiced_label = lv_label_create(pitch_container);
    lv_label_set_text(voiced_label, "UNVOICED");
    lv_obj_set_style_text_color(voiced_label, lv_color_make(0x707070), 0);
    lv_obj_set_style_text_font(voiced_label, &lv_font_montserrat_14, 0);
    
    // === EFFECT BUTTONS ===
    lv_obj_t* effects_container = lv_obj_create(performance_container);
    lv_obj_set_size(effects_container, LV_PCT(100), 56);
    lv_obj_set_style_bg_color(effects_container, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_border_width(effects_container, 0, 0);
    lv_obj_set_style_pad_column(effects_container, 8, 0);
    lv_obj_set_flex_flow(effects_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(effects_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    for (int i = 0; i < 4; i++) {
        effect_buttons[i] = lv_btn_create(effects_container);
        lv_obj_set_size(effect_buttons[i], 70, 50);
        lv_obj_set_style_bg_color(effect_buttons[i], lv_color_make(0x424242), 0);  // OFF state
        lv_obj_set_style_radius(effect_buttons[i], 8, 0);
        
        lv_obj_t* btn_label = lv_label_create(effect_buttons[i]);
        lv_label_set_text(btn_label, effect_names[i]);
        lv_obj_center(btn_label);
        lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_12, 0);
    }
    
    // === FOOTSWITCH LABELS ===
    lv_obj_t* fs_container = lv_obj_create(performance_container);
    lv_obj_set_size(fs_container, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(fs_container, lv_color_make(0x2A, 0x2A, 0x3A), 0);
    lv_obj_set_style_radius(fs_container, 0, 0);
    lv_obj_set_style_border_width(fs_container, 0, 0);
    lv_obj_set_style_pad_all(fs_container, 4, 0);
    lv_obj_set_flex_flow(fs_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    fs_labels[0] = lv_label_create(fs_container);
    lv_label_set_text(fs_labels[0], "FS1 HARMONY");
    lv_obj_set_style_text_color(fs_labels[0], lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs_labels[0], &lv_font_montserrat_12, 0);
    
    fs_labels[1] = lv_label_create(fs_container);
    lv_label_set_text(fs_labels[1], "FS2 REVERB");
    lv_obj_set_style_text_color(fs_labels[1], lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs_labels[1], &lv_font_montserrat_12, 0);
}

void performance_update_meters(float inputDb, float outputDb) {
    if (input_meter) lv_bar_set_value(input_meter, (int32_t)inputDb, LV_ANIM_ON);
    if (output_meter) lv_bar_set_value(output_meter, (int32_t)outputDb, LV_ANIM_ON);
}

void performance_update_pitch(float freqHz, const char* noteName, bool voiced) {
    if (pitch_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Pitch: %s  %.1fHz", noteName, freqHz);
        lv_label_set_text(pitch_label, buf);
    }
    if (voiced_label) {
        lv_label_set_text(voiced_label, voiced ? "VOICED" : "UNVOICED");
        lv_obj_set_style_text_color(voiced_label, voiced ? lv_color_make(0x4CAF50) : lv_color_make(0x707070), 0);
    }
}

void performance_update_effect(int effectIndex, bool enabled) {
    if (effectIndex >= 0 && effectIndex < 4 && effect_buttons[effectIndex]) {
        lv_obj_set_style_bg_color(effect_buttons[effectIndex], 
            enabled ? lv_color_make(0x4CAF50) : lv_color_make(0x424242), 0);
    }
}

void performance_update_preset(const char* name) {
    if (preset_label) lv_label_set_text(preset_label, name);
}

void performance_update_link(bool connected) {
    if (link_indicator) {
        lv_label_set_text(link_indicator, connected ? "● LINK" : "○ LINK");
        lv_obj_set_style_text_color(link_indicator, 
            connected ? lv_color_make(0x66BB6A) : lv_color_make(0xEF5350), 0);
    }
}

void performance_update_footswitch(int fsIndex, const char* label, bool pressed) {
    if (fsIndex >= 0 && fsIndex < 2 && fs_labels[fsIndex]) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s%s", label, pressed ? " [PRESSED]" : "");
        lv_label_set_text(fs_labels[fsIndex], buf);
        lv_obj_set_style_text_color(fs_labels[fsIndex],
            pressed ? lv_color_make(0xFFA726) : lv_color_make(0xB0B0B0), 0);
    }
}
