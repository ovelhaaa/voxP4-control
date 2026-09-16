#include "PresetsScreen.h"
#include "../UiTheme.h"
#include <cstdio>

static lv_obj_t* presets_container = nullptr;
static lv_obj_t* preset_list = nullptr;
static lv_obj_t* current_preset_label = nullptr;
static lv_obj_t* load_btn = nullptr;
static lv_obj_t* save_btn = nullptr;
static lv_obj_t* delete_btn = nullptr;

static int selected_index = -1;
static int current_preset_id = 0;

void presets_screen_init(lv_obj_t* parent) {
    // Create main container with padding
    presets_container = lv_obj_create(parent);
    lv_obj_set_size(presets_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(presets_container, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_pad_all(presets_container, 8, 0);
    lv_obj_set_style_border_width(presets_container, 0, 0);
    lv_obj_set_flex_flow(presets_container, LV_FLEX_FLOW_COLUMN);
    
    // === HEADER ===
    lv_obj_t* header = lv_obj_create(presets_container);
    lv_obj_set_size(header, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(header, lv_color_make(0x2A, 0x2A, 0x3A), 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "PRESET MANAGER");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    
    // === CURRENT PRESET ===
    lv_obj_t* current_row = lv_obj_create(presets_container);
    lv_obj_set_size(current_row, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(current_row, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_radius(current_row, 8, 0);
    lv_obj_set_style_border_width(current_row, 0, 0);
    lv_obj_set_style_pad_all(current_row, 8, 0);
    lv_obj_set_flex_flow(current_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(current_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* current_title = lv_label_create(current_row);
    lv_label_set_text(current_title, "CURRENT:");
    lv_obj_set_style_text_color(current_title, lv_color_make(0xB0, 0xB0, 0xB0), 0);
    lv_obj_set_style_text_font(current_title, &lv_font_montserrat_12, 0);
    
    current_preset_label = lv_label_create(current_row);
    lv_label_set_text(current_preset_label, "P03 Lead Air");
    lv_obj_set_style_text_color(current_preset_label, lv_color_make(0x64, 0xB5, 0xF6), 0);
    lv_obj_set_style_text_font(current_preset_label, &lv_font_montserrat_14, 0);
    
    // === PRESET LIST ===
    preset_list = lv_list_create(presets_container);
    lv_obj_set_size(preset_list, LV_PCT(100), 120);
    lv_obj_set_style_bg_color(preset_list, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_radius(preset_list, 8, 0);
    lv_obj_set_style_border_width(preset_list, 0, 0);
    lv_obj_set_style_pad_all(preset_list, 4, 0);
    
    // Sample preset list (will be populated dynamically)
    const char* sample_presets[] = {
        "P01 Clean Warm",
        "P02 Bright Chorus",
        "P03 Lead Air",
        "P04 Heavy Harmony",
        "P05 Ambient Pad",
        "P06 Studio Vocal",
        "P07 Live Boost",
        "P08 Soft Reverb"
    };
    
    for (int i = 0; i < 8; i++) {
        lv_obj_t* btn = lv_list_add_btn(preset_list, NULL, sample_presets[i]);
        lv_obj_set_style_text_color(btn, lv_color_white(), 0);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_12, 0);
        lv_obj_set_style_bg_color(btn, lv_color_make(0x2A, 0x2A, 0x3A), 0);
        lv_obj_set_style_radius(btn, 4, 0);
        
        // Add event handler for selection
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            lv_obj_t* btn = lv_event_get_target(e);
            const char* name = lv_list_get_btn_text(preset_list, btn);
            selected_index = lv_obj_get_index(btn) - 1; // -1 because of possible scrollbar
            
            // Highlight selected
            lv_obj_set_style_bg_color(btn, lv_color_make(0x64, 0xB5, 0xF6), 0);
            lv_obj_set_style_text_color(btn, lv_color_black(), 0);
        }, LV_EVENT_CLICKED, NULL);
    }
    
    // === ACTION BUTTONS ===
    lv_obj_t* action_row = lv_obj_create(presets_container);
    lv_obj_set_size(action_row, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(action_row, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_border_width(action_row, 0, 0);
    lv_obj_set_style_pad_column(action_row, 8, 0);
    lv_obj_set_flex_flow(action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(action_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // LOAD button
    load_btn = lv_btn_create(action_row);
    lv_obj_set_size(load_btn, 90, 36);
    lv_obj_set_style_bg_color(load_btn, lv_color_make(0x4C, 0xAF, 0x50), 0);
    lv_obj_set_style_radius(load_btn, 8, 0);
    
    lv_obj_t* load_label = lv_label_create(load_btn);
    lv_label_set_text(load_label, "LOAD");
    lv_obj_center(load_label);
    lv_obj_set_style_text_color(load_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(load_label, &lv_font_montserrat_12, 0);
    
    // SAVE button
    save_btn = lv_btn_create(action_row);
    lv_obj_set_size(save_btn, 90, 36);
    lv_obj_set_style_bg_color(save_btn, lv_color_make(0x21, 0x96, 0xF3), 0);
    lv_obj_set_style_radius(save_btn, 8, 0);
    
    lv_obj_t* save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, "SAVE AS");
    lv_obj_center(save_label);
    lv_obj_set_style_text_color(save_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(save_label, &lv_font_montserrat_12, 0);
    
    // DELETE button
    delete_btn = lv_btn_create(action_row);
    lv_obj_set_size(delete_btn, 90, 36);
    lv_obj_set_style_bg_color(delete_btn, lv_color_make(0xEF, 0x53, 0x50), 0);
    lv_obj_set_style_radius(delete_btn, 8, 0);
    
    lv_obj_t* delete_label = lv_label_create(delete_btn);
    lv_label_set_text(delete_label, "DELETE");
    lv_obj_center(delete_label);
    lv_obj_set_style_text_color(delete_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(delete_label, &lv_font_montserrat_12, 0);
}

void presets_update_list(const char** presetNames, int count) {
    if (!preset_list) return;
    
    // Clear existing list
    lv_obj_clean(preset_list);
    
    // Add new presets
    for (int i = 0; i < count; i++) {
        lv_obj_t* btn = lv_list_add_btn(preset_list, NULL, presetNames[i]);
        lv_obj_set_style_text_color(btn, lv_color_white(), 0);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_12, 0);
        lv_obj_set_style_bg_color(btn, lv_color_make(0x2A, 0x2A, 0x3A), 0);
        lv_obj_set_style_radius(btn, 4, 0);
    }
}

void presets_select_preset(int index) {
    selected_index = index;
    // Visual feedback will be handled by individual button events
}

void presets_update_current(int currentId, const char* currentName) {
    current_preset_id = currentId;
    if (current_preset_label && currentName) {
        char buf[64];
        snprintf(buf, sizeof(buf), "P%03d %s", currentId, currentName);
        lv_label_set_text(current_preset_label, buf);
    }
}
