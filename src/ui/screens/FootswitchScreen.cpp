#include "FootswitchScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"
#include "control/FootswitchManager.h"
#include <cstdio>
#include <cstring>

using namespace VoxUiTheme;

static lv_obj_t* fs_cards[2] = {nullptr};
static lv_obj_t* fs_leds[2] = {nullptr};
static lv_obj_t* fs_state_labels[2] = {nullptr};
static lv_obj_t* fs_mode_labels[2] = {nullptr};
static lv_obj_t* fs_action_labels[2] = {nullptr};
static lv_obj_t* fs_edit_buttons[2] = {nullptr};

// Editor state.
static lv_obj_t* edit_panel = nullptr;
static lv_obj_t* edit_title = nullptr;
static lv_obj_t* edit_value_labels[5] = {nullptr};
static int edit_fs_index = -1;
static uint8_t edit_values[2][5] = {{0}};

static constexpr int kFieldCount = 5;

static const char* field_names[kFieldCount] = {"MODE", "PRESS", "RELEASE",
                                               "LONG PRESS", "DOUBLE PRESS"};

// Small labelled column used for the MODE / ACTION groups on the right side.
static lv_obj_t* create_field(lv_obj_t* parent, const char* label, const char* value,
                              lv_color_t value_color) {
    lv_obj_t* col = lv_obj_create(parent);
    lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_set_style_pad_row(col, 1, 0);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* caption = lv_label_create(col);
    lv_label_set_text(caption, label);
    lv_obj_set_style_text_color(caption, COLOR_TEXT_FAINT, 0);
    lv_obj_set_style_text_font(caption, FONT_TINY, 0);

    lv_obj_t* value_label = lv_label_create(col);
    lv_label_set_text(value_label, value);
    lv_obj_set_style_text_color(value_label, value_color, 0);
    lv_obj_set_style_text_font(value_label, FONT_SMALL, 0);
    return value_label;
}

static const char* field_value_name(int fsIndex, int field) {
    const uint8_t v = edit_values[fsIndex][field];
    if (field == 0) return footswitch_mode_name(v);
    return footswitch_action_name((FootswitchAction)v);
}

static void refresh_edit_values(void) {
    for (int f = 0; f < kFieldCount; ++f) {
        if (!edit_value_labels[f]) continue;
        if (edit_fs_index < 0) {
            lv_label_set_text(edit_value_labels[f], "--");
            continue;
        }
        lv_label_set_text(edit_value_labels[f], field_value_name(edit_fs_index, f));
    }
    if (edit_title) {
        if (edit_fs_index < 0) {
            lv_label_set_text(edit_title, "TAP EDIT TO ASSIGN");
        } else {
            char buf[24];
            snprintf(buf, sizeof(buf), "EDIT FS%d", edit_fs_index + 1);
            lv_label_set_text(edit_title, buf);
        }
    }
}

static int field_count_for(int field) {
    return field == 0 ? footswitch_mode_count() : footswitch_action_count();
}

static void edit_step_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* row = lv_obj_get_parent(btn);
    const int field = (int)(intptr_t)lv_obj_get_user_data(row);
    const int delta = (int)(intptr_t)lv_obj_get_user_data(btn);
    if (edit_fs_index < 0 || field < 0 || field >= kFieldCount) return;

    const int count = field_count_for(field);
    int value = (int)edit_values[edit_fs_index][field];
    value += delta;
    if (value < 0) value = count - 1;
    if (value >= count) value = 0;

    ui_set_footswitch_field(edit_fs_index, (UiFootswitchField)field, (uint8_t)value);
    refresh_edit_values();
}

static void edit_button_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const int index = (int)(intptr_t)lv_obj_get_user_data(btn);
    edit_fs_index = (edit_fs_index == index) ? -1 : index;
    for (int i = 0; i < 2; ++i) {
        if (fs_edit_buttons[i]) {
            lv_obj_set_style_bg_color(fs_edit_buttons[i],
                (edit_fs_index == i) ? COLOR_ACCENT_DARK : COLOR_PANEL, 0);
        }
    }
    refresh_edit_values();
}

static lv_obj_t* create_edit_row(lv_obj_t* parent, const char* label, int field) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), 34);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(row, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(row, RADIUS_S, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_hor(row, SPACING_S, 0);
    lv_obj_set_style_pad_ver(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_user_data(row, (void*)(intptr_t)field);

    lv_obj_t* name = lv_label_create(row);
    lv_label_set_text(name, label);
    lv_obj_set_style_text_color(name, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(name, FONT_TINY, 0);

    lv_obj_t* controls = lv_obj_create(row);
    lv_obj_set_size(controls, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_clear_flag(controls, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(controls, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(controls, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(controls, 0, 0);
    lv_obj_set_style_pad_all(controls, 0, 0);
    lv_obj_set_style_pad_column(controls, SPACING_XS, 0);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t* minus = lv_btn_create(controls);
    lv_obj_set_size(minus, 36, 24);
    lv_obj_set_ext_click_area(minus, 4);
    lv_obj_set_style_radius(minus, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(minus, 0, 0);
    lv_obj_set_style_border_width(minus, 1, 0);
    lv_obj_set_style_border_color(minus, COLOR_SEPARATOR, 0);
    lv_obj_set_style_bg_color(minus, COLOR_PANEL, 0);
    ui_apply_pressed(minus, COLOR_SURFACE_ELEV, COLOR_ACCENT);
    lv_obj_t* minus_label = lv_label_create(minus);
    lv_label_set_text(minus_label, "<");
    lv_obj_center(minus_label);
    lv_obj_set_style_text_color(minus_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_user_data(minus, (void*)(intptr_t)-1);
    lv_obj_add_event_cb(minus, edit_step_event, LV_EVENT_CLICKED, NULL);

    lv_obj_t* value = lv_label_create(controls);
    lv_label_set_text(value, "--");
    lv_obj_set_width(value, 92);
    lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(value, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(value, FONT_TINY, 0);
    edit_value_labels[field] = value;

    lv_obj_t* plus = lv_btn_create(controls);
    lv_obj_set_size(plus, 36, 24);
    lv_obj_set_ext_click_area(plus, 4);
    lv_obj_set_style_radius(plus, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(plus, 0, 0);
    lv_obj_set_style_border_width(plus, 1, 0);
    lv_obj_set_style_border_color(plus, COLOR_SEPARATOR, 0);
    lv_obj_set_style_bg_color(plus, COLOR_PANEL, 0);
    ui_apply_pressed(plus, COLOR_SURFACE_ELEV, COLOR_ACCENT);
    lv_obj_t* plus_label = lv_label_create(plus);
    lv_label_set_text(plus_label, ">");
    lv_obj_center(plus_label);
    lv_obj_set_style_text_color(plus_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_user_data(plus, (void*)(intptr_t)1);
    lv_obj_add_event_cb(plus, edit_step_event, LV_EVENT_CLICKED, NULL);

    return row;
}

void footswitch_screen_init(lv_obj_t* parent) {
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(container, SPACING_XS, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(container, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_AUTO);

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
    lv_label_set_text(title, "FOOTSWITCH");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_BODY, 0);

    // === FS1 / FS2 MODULES ===
    for (int i = 0; i < 2; i++) {
        lv_obj_t* card = lv_obj_create(container);
        lv_obj_set_size(card, LV_PCT(100), 70);
        lv_obj_set_style_bg_color(card, COLOR_SURFACE, 0);
        lv_obj_set_style_radius(card, RADIUS_M, 0);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_border_color(card, COLOR_SEPARATOR, 0);
        lv_obj_set_style_pad_all(card, SPACING_M, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Left: LED + FS number + state
        lv_obj_t* left = lv_obj_create(card);
        lv_obj_set_size(left, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_clear_flag(left, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(left, 0, 0);
        lv_obj_set_style_pad_all(left, 0, 0);
        lv_obj_set_style_pad_row(left, 2, 0);
        lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(left, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        lv_obj_t* left_top = lv_obj_create(left);
        lv_obj_set_size(left_top, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_clear_flag(left_top, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(left_top, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(left_top, 0, 0);
        lv_obj_set_style_pad_all(left_top, 0, 0);
        lv_obj_set_style_pad_column(left_top, SPACING_XS, 0);
        lv_obj_set_flex_flow(left_top, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(left_top, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t* led = lv_obj_create(left_top);
        lv_obj_set_size(led, 8, 8);
        lv_obj_clear_flag(led, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(led, COLOR_DISABLED, 0);
        lv_obj_set_style_radius(led, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(led, 0, 0);
        lv_obj_set_style_pad_all(led, 0, 0);

        char num[8];
        snprintf(num, sizeof(num), "FS%d", i + 1);
        lv_obj_t* num_label = lv_label_create(left_top);
        lv_label_set_text(num_label, num);
        lv_obj_set_style_text_color(num_label, COLOR_TEXT_PRIMARY, 0);
        lv_obj_set_style_text_font(num_label, FONT_EMPHASIS, 0);

        lv_obj_t* state_label = lv_label_create(left);
        lv_label_set_text(state_label, "RELEASED");
        lv_obj_set_style_text_color(state_label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(state_label, FONT_TINY, 0);

        // Middle: MODE / ACTION groups
        lv_obj_t* mid = lv_obj_create(card);
        lv_obj_set_size(mid, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_clear_flag(mid, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(mid, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(mid, 0, 0);
        lv_obj_set_style_pad_all(mid, 0, 0);
        lv_obj_set_style_pad_column(mid, SPACING_M, 0);
        lv_obj_set_flex_flow(mid, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(mid, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t* mode_label = create_field(mid, "MODE", "--", COLOR_TEXT_MUTED);
        lv_obj_t* action_label = create_field(mid, "ACTION", "--", COLOR_TEXT_MUTED);

        // Right: EDIT toggle
        lv_obj_t* edit_btn = lv_btn_create(card);
        lv_obj_set_size(edit_btn, 44, 26);
        lv_obj_set_ext_click_area(edit_btn, 6);
        lv_obj_set_style_radius(edit_btn, RADIUS_S, 0);
        lv_obj_set_style_border_width(edit_btn, 1, 0);
        lv_obj_set_style_border_color(edit_btn, COLOR_SEPARATOR, 0);
        lv_obj_set_style_bg_color(edit_btn, COLOR_PANEL, 0);
        lv_obj_set_style_shadow_width(edit_btn, 0, 0);
        ui_apply_pressed(edit_btn, COLOR_SURFACE_ELEV, COLOR_ACCENT);
        lv_obj_set_user_data(edit_btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(edit_btn, edit_button_event, LV_EVENT_CLICKED, NULL);
        lv_obj_t* edit_label = lv_label_create(edit_btn);
        lv_label_set_text(edit_label, "EDIT");
        lv_obj_center(edit_label);
        lv_obj_set_style_text_color(edit_label, COLOR_TEXT_SECONDARY, 0);
        lv_obj_set_style_text_font(edit_label, FONT_TINY, 0);

        fs_cards[i] = card;
        fs_leds[i] = led;
        fs_state_labels[i] = state_label;
        fs_mode_labels[i] = mode_label;
        fs_action_labels[i] = action_label;
        fs_edit_buttons[i] = edit_btn;
    }

    // === EDIT PANEL (secondary options hidden until EDIT is tapped) ===
    edit_panel = lv_obj_create(container);
    lv_obj_set_size(edit_panel, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(edit_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(edit_panel, COLOR_BG, 0);
    lv_obj_set_style_border_width(edit_panel, 0, 0);
    lv_obj_set_style_pad_all(edit_panel, 0, 0);
    lv_obj_set_style_pad_row(edit_panel, SPACING_XS, 0);
    lv_obj_set_flex_flow(edit_panel, LV_FLEX_FLOW_COLUMN);

    edit_title = lv_label_create(edit_panel);
    lv_label_set_text(edit_title, "TAP EDIT TO ASSIGN");
    lv_obj_set_style_text_color(edit_title, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(edit_title, FONT_TINY, 0);

    for (int f = 0; f < kFieldCount; ++f) {
        create_edit_row(edit_panel, field_names[f], f);
    }
    refresh_edit_values();
}

void footswitch_update_config(int fsIndex, uint8_t mode, const char* actionName) {
    if (fsIndex < 0 || fsIndex > 1) return;
    if (mode < 2 && fs_mode_labels[fsIndex]) {
        lv_label_set_text(fs_mode_labels[fsIndex], footswitch_mode_name(mode));
    }
    if (actionName && fs_action_labels[fsIndex]) {
        lv_label_set_text(fs_action_labels[fsIndex], actionName);
    }
}

void footswitch_update_state(int fsIndex, bool pressed) {
    if (fsIndex < 0 || fsIndex > 1) return;

    if (fs_state_labels[fsIndex]) {
        lv_label_set_text(fs_state_labels[fsIndex], pressed ? "PRESSED" : "RELEASED");
        lv_obj_set_style_text_color(fs_state_labels[fsIndex],
            pressed ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_MUTED, 0);
    }
    if (fs_leds[fsIndex]) {
        lv_obj_set_style_bg_color(fs_leds[fsIndex], pressed ? COLOR_ACCENT : COLOR_DISABLED, 0);
    }
    if (fs_cards[fsIndex]) {
        lv_obj_set_style_border_color(fs_cards[fsIndex],
            pressed ? COLOR_ACCENT : COLOR_SEPARATOR, 0);
    }
}

void footswitch_update_edit(int fsIndex, uint8_t mode, uint8_t press,
                            uint8_t release, uint8_t longPress,
                            uint8_t doublePress) {
    if (fsIndex < 0 || fsIndex > 1) return;
    edit_values[fsIndex][0] = mode;
    edit_values[fsIndex][1] = press;
    edit_values[fsIndex][2] = release;
    edit_values[fsIndex][3] = longPress;
    edit_values[fsIndex][4] = doublePress;
    if (fsIndex == edit_fs_index) refresh_edit_values();
}
