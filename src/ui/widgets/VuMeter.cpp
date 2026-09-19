/**
 * VoxP4 CYD - VU Meter Widget Implementation
 * Horizontal meter otimizado para 320x240.
 * Turquoise = signal, orange = warning, red = clip.
 */

#include "VuMeter.h"
#include "../UiTheme.h"
#include <stdio.h>

// Converte dB (-60 a 0) para posição da barra (0-100%)
static int32_t db_to_percent(float db) {
    if (db <= -60.0f) return 0;
    if (db >= 0.0f) return 100;
    float normalized = (db + 60.0f) / 60.0f;
    return (int32_t)(normalized * 100.0f);
}

// Clip threshold and hold. The bar itself stays turquoise; warning/clip is
// shown by the dedicated marker instead of recolouring the whole bar.
static constexpr float kClipThresholdDb = -1.0f;
static constexpr uint32_t kClipHoldMs = 800;

VuMeter_t* vu_meter_create(lv_obj_t* parent, int32_t x, int32_t y, 
                           int32_t height, const char* label_text) {
    VuMeter_t* meter = (VuMeter_t*)lv_mem_alloc(sizeof(VuMeter_t));
    if (!meter) return NULL;
    
    meter->container = lv_obj_create(parent);
    lv_obj_set_size(meter->container, 180, height);
    lv_obj_set_pos(meter->container, x, y);
    lv_obj_set_style_bg_color(meter->container, COLOR_PANEL, 0);
    lv_obj_set_style_border_width(meter->container, 0, 0);
    lv_obj_set_style_radius(meter->container, RADIUS_S, 0);
    lv_obj_set_style_pad_all(meter->container, 2, 0);
    lv_obj_set_style_pad_column(meter->container, SPACING_XS, 0);
    lv_obj_set_flex_flow(meter->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(meter->container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    meter->bar = lv_bar_create(meter->container);
    lv_obj_set_height(meter->bar, height - 4);
    lv_obj_set_flex_grow(meter->bar, 1);
    lv_bar_set_range(meter->bar, 0, 100);
    lv_bar_set_value(meter->bar, 0, LV_ANIM_OFF);
    
    // Track
    lv_obj_set_style_bg_color(meter->bar, COLOR_BG, 0);
    lv_obj_set_style_radius(meter->bar, RADIUS_S, 0);
    lv_obj_set_style_border_width(meter->bar, 0, 0);
    
    // Indicator
    lv_obj_set_style_bg_color(meter->bar, COLOR_AUDIO, LV_PART_INDICATOR);
    lv_obj_set_style_radius(meter->bar, RADIUS_S, LV_PART_INDICATOR);
    
    meter->db_label = lv_label_create(meter->container);
    lv_obj_set_width(meter->db_label, 32);
    lv_label_set_long_mode(meter->db_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(meter->db_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(meter->db_label, "-60");
    lv_obj_set_style_text_font(meter->db_label, FONT_TINY, 0);
    lv_obj_set_style_text_color(meter->db_label, COLOR_TEXT_SECONDARY, 0);

    // Small clip marker at the far right; off unless the signal clips.
    meter->clip_led = lv_obj_create(meter->container);
    lv_obj_set_size(meter->clip_led, 5, 5);
    lv_obj_clear_flag(meter->clip_led, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(meter->clip_led, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(meter->clip_led, 0, 0);
    lv_obj_set_style_pad_all(meter->clip_led, 0, 0);
    lv_obj_set_style_bg_color(meter->clip_led, COLOR_DISABLED, 0);
    
    meter->current_db = -60.0f;
    meter->peak_db = -60.0f;
    meter->peak_hold_time = 0;
    meter->clip_hold_until = 0;
    
    (void)x; (void)y; (void)label_text;
    
    return meter;
}

lv_obj_t* vu_meter_get_container(VuMeter_t* meter) {
    if (!meter) return NULL;
    return meter->container;
}

void vu_meter_update(VuMeter_t* meter, float db) {
    if (!meter) return;
    
    if (db < -60.0f) db = -60.0f;
    if (db > 0.0f) db = 0.0f;
    
    meter->current_db = db;
    
    int32_t percent = db_to_percent(db);
    lv_bar_set_value(meter->bar, percent, LV_ANIM_OFF);
    
    // Normal audio stays turquoise; the whole bar no longer turns red.
    lv_obj_set_style_bg_color(meter->bar, COLOR_AUDIO, LV_PART_INDICATOR);
    
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", (int)(db + (db > 0 ? 0.5f : -0.5f)));
    lv_label_set_text(meter->db_label, buf);

    // Clip marker with a short hold so single-sample peaks stay visible.
    const uint32_t now = lv_tick_get();
    if (db >= kClipThresholdDb) {
        meter->clip_hold_until = now + kClipHoldMs;
    }
    const bool clipping = (int32_t)(now - meter->clip_hold_until) < 0;
    if (meter->clip_led) {
        lv_obj_set_style_bg_color(meter->clip_led,
            clipping ? COLOR_ERROR : COLOR_DISABLED, 0);
    }
}

void vu_meter_reset_peak(VuMeter_t* meter) {
    if (!meter) return;
    meter->peak_db = -60.0f;
}
