#include "PresetsScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"
#include <cstdio>
#include <cstring>

#define PRESET_MAX_ROWS 16

static lv_obj_t* preset_list = nullptr;
static lv_obj_t* current_preset_label = nullptr;

static lv_obj_t* preset_rows[PRESET_MAX_ROWS] = {nullptr};
static lv_obj_t* preset_bars[PRESET_MAX_ROWS] = {nullptr};
static lv_obj_t* preset_names[PRESET_MAX_ROWS] = {nullptr};
static int preset_row_count = 0;
static int selected_index = -1;
static int current_preset_id = 0;

static void apply_selection(int index) {
    selected_index = index;
    for (int i = 0; i < preset_row_count; i++) {
        bool sel = (i == index);
        if (preset_rows[i]) {
            lv_obj_set_style_bg_color(preset_rows[i], sel ? COLOR_SURFACE_ELEV : COLOR_SURFACE, 0);
            lv_obj_set_style_border_color(preset_rows[i], sel ? COLOR_ACCENT_DARK : COLOR_SEPARATOR, 0);
        }
        if (preset_bars[i]) {
            if (sel) lv_obj_clear_flag(preset_bars[i], LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(preset_bars[i], LV_OBJ_FLAG_HIDDEN);
        }
        if (preset_names[i]) {
            lv_obj_set_style_text_color(preset_names[i], sel ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_SECONDARY, 0);
        }
    }
}

static void preset_row_clicked(lv_event_t* e) {
    lv_obj_t* row = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_obj_get_user_data(row);
    apply_selection(index);
}

static lv_obj_t* add_preset_row(int index, const char* name) {
    if (index < 0 || index >= PRESET_MAX_ROWS) return nullptr;

    lv_obj_t* row = lv_obj_create(preset_list);
    lv_obj_set_size(row, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(row, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(row, RADIUS_S, 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_color(row, COLOR_SEPARATOR, 0);
    lv_obj_set_style_pad_hor(row, SPACING_M, 0);
    lv_obj_set_style_pad_ver(row, 0, 0);
    lv_obj_set_style_pad_column(row, SPACING_S, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(row, (void*)(intptr_t)index);
    lv_obj_add_event_cb(row, preset_row_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t* bar = lv_obj_create(row);
    lv_obj_set_size(bar, 3, 14);
    lv_obj_align(bar, LV_ALIGN_LEFT_MID, -2, 0);
    lv_obj_set_style_bg_color(bar, COLOR_ACCENT, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, RADIUS_S, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);

    char num[4];
    snprintf(num, sizeof(num), "%02d", index + 1);
    lv_obj_t* num_label = lv_label_create(row);
    lv_label_set_text(num_label, num);
    lv_obj_set_style_text_color(num_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(num_label, FONT_TINY, 0);
    lv_obj_set_width(num_label, 18);

    lv_obj_t* name_label = lv_label_create(row);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_color(name_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(name_label, FONT_SMALL, 0);
    lv_obj_set_flex_grow(name_label, 1);

    preset_rows[index] = row;
    preset_bars[index] = bar;
    preset_names[index] = name_label;
    return row;
}

static lv_obj_t* create_action_button(lv_obj_t* parent, const char* text, bool accent) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_height(btn, 32);
    lv_obj_set_flex_grow(btn, 1);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, accent ? COLOR_ACCENT : COLOR_SEPARATOR, 0);
    lv_obj_set_style_radius(btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(label, accent ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_SECONDARY, 0);
    return btn;
}

void presets_screen_init(lv_obj_t* parent) {
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(container, SPACING_XS, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    // === HEADER ===
    lv_obj_t* header = lv_obj_create(container);
    lv_obj_set_size(header, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_radius(header, RADIUS_S, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_style_pad_ver(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "PRESETS");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_BODY, 0);

    // === CURRENT PRESET ===
    lv_obj_t* current_row = lv_obj_create(container);
    lv_obj_set_size(current_row, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(current_row, COLOR_SURFACE_ELEV, 0);
    lv_obj_set_style_radius(current_row, RADIUS_S, 0);
    lv_obj_set_style_border_width(current_row, 1, 0);
    lv_obj_set_style_border_color(current_row, COLOR_ACCENT_DARK, 0);
    lv_obj_set_style_pad_hor(current_row, SPACING_M, 0);
    lv_obj_set_flex_flow(current_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(current_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* current_title = lv_label_create(current_row);
    lv_label_set_text(current_title, "CURRENT");
    lv_obj_set_style_text_color(current_title, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(current_title, FONT_TINY, 0);

    current_preset_label = lv_label_create(current_row);
    lv_label_set_text(current_preset_label, "P--  --");
    lv_obj_set_style_text_color(current_preset_label, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(current_preset_label, FONT_SMALL, 0);

    // === PRESET LIST (scrollable) ===
    preset_list = lv_obj_create(container);
    lv_obj_set_size(preset_list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(preset_list, 1);
    lv_obj_set_style_bg_color(preset_list, COLOR_BG, 0);
    lv_obj_set_style_border_width(preset_list, 0, 0);
    lv_obj_set_style_pad_all(preset_list, 0, 0);
    lv_obj_set_style_pad_row(preset_list, 2, 0);
    lv_obj_set_flex_flow(preset_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(preset_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(preset_list, LV_SCROLLBAR_MODE_AUTO);

    static const char* sample_presets[] = {
        "CLEAN WARM", "BRIGHT CHORUS", "LEAD AIR", "HEAVY HARMONY",
        "AMBIENT PAD", "STUDIO VOCAL", "LIVE BOOST", "SOFT REVERB"
    };
    for (int i = 0; i < 8; i++) {
        add_preset_row(preset_row_count++, sample_presets[i]);
    }
    apply_selection(2);

    // === ACTIONS (secondary to the list) ===
    lv_obj_t* action_row = lv_obj_create(container);
    lv_obj_set_size(action_row, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(action_row, COLOR_BG, 0);
    lv_obj_set_style_border_width(action_row, 0, 0);
    lv_obj_set_style_pad_all(action_row, 0, 0);
    lv_obj_set_style_pad_column(action_row, SPACING_XS, 0);
    lv_obj_set_flex_flow(action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(action_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* load_btn = create_action_button(action_row, "LOAD", true);
    lv_obj_add_event_cb(load_btn, [](lv_event_t* e) {
        UiAction action = { UiActionType::LoadPreset, (uint16_t)(selected_index + 1), 0 };
        ui_emit_action(action);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* save_btn = create_action_button(action_row, "SAVE AS", false);
    lv_obj_add_event_cb(save_btn, [](lv_event_t* e) {
        UiAction action = { UiActionType::SavePreset, (uint16_t)(selected_index + 1), 0 };
        ui_emit_action(action);
    }, LV_EVENT_CLICKED, NULL);

    create_action_button(action_row, "DELETE", false);
}

void presets_update_list(const char** presetNames, int count) {
    if (!preset_list) return;
    if (count > PRESET_MAX_ROWS) count = PRESET_MAX_ROWS;

    lv_obj_clean(preset_list);
    for (int i = 0; i < PRESET_MAX_ROWS; i++) {
        preset_rows[i] = nullptr;
        preset_bars[i] = nullptr;
        preset_names[i] = nullptr;
    }
    preset_row_count = 0;
    for (int i = 0; i < count; i++) {
        add_preset_row(preset_row_count++, presetNames[i]);
    }
    apply_selection(0);
}

void presets_select_preset(int index) {
    apply_selection(index);
}

void presets_update_current(int currentId, const char* currentName) {
    current_preset_id = currentId;
    if (current_preset_label && currentName) {
        char buf[48];
        snprintf(buf, sizeof(buf), "P%02d  %s", currentId, currentName);
        lv_label_set_text(current_preset_label, buf);
    }
}
