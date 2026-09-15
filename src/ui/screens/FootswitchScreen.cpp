#include "FootswitchScreen.h"
#include "../UiTheme.h"

static lv_obj_t* fs_container = nullptr;
static lv_obj_t* fs1_mode_label = nullptr;
static lv_obj_t* fs2_mode_label = nullptr;
static lv_obj_t* fs1_action_label = nullptr;
static lv_obj_t* fs2_action_label = nullptr;
static lv_obj_t* fs1_state_indicator = nullptr;
static lv_obj_t* fs2_state_indicator = nullptr;

// Mode names
static const char* mode_names[] = {"MOMENTARY", "LATCHING"};

// Action names (subset of supported actions)
static const char* action_names[] = {
    "HARMONY TOGGLE",
    "HARMONY MOMENTARY",
    "REVERB TOGGLE",
    "REVERB FREEZE",
    "DELAY TOGGLE",
    "TAP TEMPO",
    "PRESET NEXT",
    "PRESET PREV",
    "GLOBAL BYPASS"
};

void footswitch_screen_init(lv_obj_t* parent) {
    // Create main container with padding
    fs_container = lv_obj_create(parent);
    lv_obj_set_size(fs_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(fs_container, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_pad_all(fs_container, 8, 0);
    lv_obj_set_style_border_width(fs_container, 0, 0);
    lv_obj_set_flex_flow(fs_container, LV_FLEX_FLOW_COLUMN);
    
    // === HEADER ===
    lv_obj_t* header = lv_obj_create(fs_container);
    lv_obj_set_size(header, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(header, lv_color_make(0x2A, 0x2A, 0x3A), 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "FOOTSWITCH CONFIG");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    
    // === FS1 SECTION ===
    lv_obj_t* fs1_card = lv_obj_create(fs_container);
    lv_obj_set_size(fs1_card, LV_PCT(100), 80);
    lv_obj_set_style_bg_color(fs1_card, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_radius(fs1_card, 8, 0);
    lv_obj_set_style_border_width(fs1_card, 0, 0);
    lv_obj_set_style_pad_all(fs1_card, 12, 0);
    lv_obj_set_flex_flow(fs1_card, LV_FLEX_FLOW_COLUMN);
    
    // FS1 Header
    lv_obj_t* fs1_header = lv_obj_create(fs1_card);
    lv_obj_set_size(fs1_header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(fs1_header, lv_color_make(0x2A, 0x2A, 0x3A), 0);
    lv_obj_set_style_radius(fs1_header, 6, 0);
    lv_obj_set_style_border_width(fs1_header, 0, 0);
    lv_obj_set_flex_flow(fs1_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs1_header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* fs1_title = lv_label_create(fs1_header);
    lv_label_set_text(fs1_title, "FS1");
    lv_obj_set_style_text_color(fs1_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(fs1_title, &lv_font_montserrat_14, 0);
    
    fs1_state_indicator = lv_label_create(fs1_header);
    lv_label_set_text(fs1_state_indicator, "RELEASED");
    lv_obj_set_style_text_color(fs1_state_indicator, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs1_state_indicator, &lv_font_montserrat_12, 0);
    
    // FS1 Mode
    lv_obj_t* fs1_mode_row = lv_obj_create(fs1_card);
    lv_obj_set_size(fs1_mode_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(fs1_mode_row, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_border_width(fs1_mode_row, 0, 0);
    lv_obj_set_flex_flow(fs1_mode_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs1_mode_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* fs1_mode_title = lv_label_create(fs1_mode_row);
    lv_label_set_text(fs1_mode_title, "MODE: ");
    lv_obj_set_style_text_color(fs1_mode_title, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs1_mode_title, &lv_font_montserrat_12, 0);
    
    fs1_mode_label = lv_label_create(fs1_mode_row);
    lv_label_set_text(fs1_mode_label, "MOMENTARY");
    lv_obj_set_style_text_color(fs1_mode_label, lv_color_make(0x64B5F6), 0);
    lv_obj_set_style_text_font(fs1_mode_label, &lv_font_montserrat_12, 0);
    
    // FS1 Action
    lv_obj_t* fs1_action_row = lv_obj_create(fs1_card);
    lv_obj_set_size(fs1_action_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(fs1_action_row, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_border_width(fs1_action_row, 0, 0);
    lv_obj_set_flex_flow(fs1_action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs1_action_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* fs1_action_title = lv_label_create(fs1_action_row);
    lv_label_set_text(fs1_action_title, "ACTION: ");
    lv_obj_set_style_text_color(fs1_action_title, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs1_action_title, &lv_font_montserrat_12, 0);
    
    fs1_action_label = lv_label_create(fs1_action_row);
    lv_label_set_text(fs1_action_label, "HARMONY TOGGLE");
    lv_obj_set_style_text_color(fs1_action_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(fs1_action_label, &lv_font_montserrat_12, 0);
    
    // === FS2 SECTION ===
    lv_obj_t* fs2_card = lv_obj_create(fs_container);
    lv_obj_set_size(fs2_card, LV_PCT(100), 80);
    lv_obj_set_style_bg_color(fs2_card, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_radius(fs2_card, 8, 0);
    lv_obj_set_style_border_width(fs2_card, 0, 0);
    lv_obj_set_style_pad_all(fs2_card, 12, 0);
    lv_obj_set_flex_flow(fs2_card, LV_FLEX_FLOW_COLUMN);
    
    // FS2 Header
    lv_obj_t* fs2_header = lv_obj_create(fs2_card);
    lv_obj_set_size(fs2_header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(fs2_header, lv_color_make(0x2A, 0x2A, 0x3A), 0);
    lv_obj_set_style_radius(fs2_header, 6, 0);
    lv_obj_set_style_border_width(fs2_header, 0, 0);
    lv_obj_set_flex_flow(fs2_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs2_header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* fs2_title = lv_label_create(fs2_header);
    lv_label_set_text(fs2_title, "FS2");
    lv_obj_set_style_text_color(fs2_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(fs2_title, &lv_font_montserrat_14, 0);
    
    fs2_state_indicator = lv_label_create(fs2_header);
    lv_label_set_text(fs2_state_indicator, "RELEASED");
    lv_obj_set_style_text_color(fs2_state_indicator, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs2_state_indicator, &lv_font_montserrat_12, 0);
    
    // FS2 Mode
    lv_obj_t* fs2_mode_row = lv_obj_create(fs2_card);
    lv_obj_set_size(fs2_mode_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(fs2_mode_row, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_border_width(fs2_mode_row, 0, 0);
    lv_obj_set_flex_flow(fs2_mode_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs2_mode_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* fs2_mode_title = lv_label_create(fs2_mode_row);
    lv_label_set_text(fs2_mode_title, "MODE: ");
    lv_obj_set_style_text_color(fs2_mode_title, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs2_mode_title, &lv_font_montserrat_12, 0);
    
    fs2_mode_label = lv_label_create(fs2_mode_row);
    lv_label_set_text(fs2_mode_label, "LATCHING");
    lv_obj_set_style_text_color(fs2_mode_label, lv_color_make(0x64B5F6), 0);
    lv_obj_set_style_text_font(fs2_mode_label, &lv_font_montserrat_12, 0);
    
    // FS2 Action
    lv_obj_t* fs2_action_row = lv_obj_create(fs2_card);
    lv_obj_set_size(fs2_action_row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(fs2_action_row, lv_color_make(0x1E, 0x1E, 0x2E), 0);
    lv_obj_set_style_border_width(fs2_action_row, 0, 0);
    lv_obj_set_flex_flow(fs2_action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs2_action_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* fs2_action_title = lv_label_create(fs2_action_row);
    lv_label_set_text(fs2_action_title, "ACTION: ");
    lv_obj_set_style_text_color(fs2_action_title, lv_color_make(0xB0B0B0), 0);
    lv_obj_set_style_text_font(fs2_action_title, &lv_font_montserrat_12, 0);
    
    fs2_action_label = lv_label_create(fs2_action_row);
    lv_label_set_text(fs2_action_label, "REVERB TOGGLE");
    lv_obj_set_style_text_color(fs2_action_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(fs2_action_label, &lv_font_montserrat_12, 0);
    
    // === HELP TEXT ===
    lv_obj_t* help_label = lv_label_create(fs_container);
    lv_label_set_text(help_label, "Tap to edit configuration. Long press to test.");
    lv_obj_set_style_text_color(help_label, lv_color_make(0x707070), 0);
    lv_obj_set_style_text_font(help_label, &lv_font_montserrat_10, 0);
    lv_obj_center(help_label);
}

void footswitch_update_config(int fsIndex, uint8_t mode, const char* actionName) {
    if (fsIndex == 0) {
        if (fs1_mode_label && mode < 2) {
            lv_label_set_text(fs1_mode_label, mode_names[mode]);
        }
        if (fs1_action_label && actionName) {
            lv_label_set_text(fs1_action_label, actionName);
        }
    } else if (fsIndex == 1) {
        if (fs2_mode_label && mode < 2) {
            lv_label_set_text(fs2_mode_label, mode_names[mode]);
        }
        if (fs2_action_label && actionName) {
            lv_label_set_text(fs2_action_label, actionName);
        }
    }
}

void footswitch_update_state(int fsIndex, bool pressed) {
    if (fsIndex == 0) {
        if (fs1_state_indicator) {
            lv_label_set_text(fs1_state_indicator, pressed ? "PRESSED" : "RELEASED");
            lv_obj_set_style_text_color(fs1_state_indicator, 
                pressed ? lv_color_make(0xFFA726) : lv_color_make(0xB0B0B0), 0);
        }
    } else if (fsIndex == 1) {
        if (fs2_state_indicator) {
            lv_label_set_text(fs2_state_indicator, pressed ? "PRESSED" : "RELEASED");
            lv_obj_set_style_text_color(fs2_state_indicator, 
                pressed ? lv_color_make(0xFFA726) : lv_color_make(0xB0B0B0), 0);
        }
    }
}
