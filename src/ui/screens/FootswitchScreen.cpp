#include "FootswitchScreen.h"
#include "../UiTheme.h"
#include <cstdio>

static lv_obj_t* fs_cards[2] = {nullptr};
static lv_obj_t* fs_leds[2] = {nullptr};
static lv_obj_t* fs_state_labels[2] = {nullptr};
static lv_obj_t* fs_mode_labels[2] = {nullptr};
static lv_obj_t* fs_action_labels[2] = {nullptr};

static const char* mode_names[] = {"MOMENTARY", "LATCHING"};

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

void footswitch_screen_init(lv_obj_t* parent) {
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
    lv_label_set_text(title, "FOOTSWITCH");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_BODY, 0);

    // === FS1 / FS2 MODULES ===
    for (int i = 0; i < 2; i++) {
        lv_obj_t* card = lv_obj_create(container);
        lv_obj_set_size(card, LV_PCT(100), 80);
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

        // Right: MODE / ACTION groups
        lv_obj_t* right = lv_obj_create(card);
        lv_obj_set_size(right, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_clear_flag(right, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(right, 0, 0);
        lv_obj_set_style_pad_all(right, 0, 0);
        lv_obj_set_style_pad_column(right, SPACING_L, 0);
        lv_obj_set_flex_flow(right, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(right, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t* mode_label = create_field(right, "MODE",
                                            i == 0 ? "MOMENTARY" : "LATCHING",
                                            COLOR_TEXT_SECONDARY);
        lv_obj_t* action_label = create_field(right, "ACTION",
                                              i == 0 ? "HARMONY" : "REVERB",
                                              COLOR_TEXT_PRIMARY);

        fs_cards[i] = card;
        fs_leds[i] = led;
        fs_state_labels[i] = state_label;
        fs_mode_labels[i] = mode_label;
        fs_action_labels[i] = action_label;
    }
}

void footswitch_update_config(int fsIndex, uint8_t mode, const char* actionName) {
    if (fsIndex < 0 || fsIndex > 1) return;
    if (mode < 2 && fs_mode_labels[fsIndex]) {
        lv_label_set_text(fs_mode_labels[fsIndex], mode_names[mode]);
    }
    if (actionName && fs_action_labels[fsIndex]) {
        lv_label_set_text(fs_action_labels[fsIndex], actionName);
    }
}

void footswitch_update_state(int fsIndex, bool pressed) {
    if (fsIndex < 0 || fsIndex > 1) return;

    // Transient press: accent border + LED + state. No full-card fill.
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
