/**
 * VoxP4 CYD - VU Meter Widget Implementation
 * 
 * Meter HORIZONTAL otimizado para Performance Screen 320x240
 * - Fast attack, controlled release
 * - Peak hold visual
 * - Zones coloridas (safe/warning/clip)
 * - Sem animações empilhadas para telemetria 20-30Hz
 */

#include "VuMeter.h"
#include "../UiTheme.h"
#include <stdio.h>

// Converte dB (-60 a 0) para pixels na barra horizontal
static int32_t db_to_pixels(float db, int32_t max_width) {
    if (db <= -60.0f) return 0;
    if (db >= 0.0f) return max_width;
    // Mapeamento logarítmico simples: mais resolução perto de 0dB
    float normalized = (db + 60.0f) / 60.0f;
    return (int32_t)(normalized * max_width);
}

// Seleciona cor baseada no nível de dB usando theme colors
static lv_color_t get_meter_color(float db) {
    if (db >= -3.0f) {
        return COLOR_METER_CLIP;
    } else if (db >= -12.0f) {
        return COLOR_METER_WARNING;
    } else {
        return COLOR_METER_SAFE;
    }
}

VuMeter_t* vu_meter_create(lv_obj_t* parent, int32_t x, int32_t y, 
                           int32_t width, int32_t height, const char* label_text) {
    VuMeter_t* meter = (VuMeter_t*)lv_calloc(1, sizeof(VuMeter_t));
    if (!meter) return NULL;
    
    meter->width = width;
    meter->height = height;
    
    // Container principal com flex row
    meter->container = lv_obj_create(parent);
    lv_obj_set_size(meter->container, width, height);
    lv_obj_set_pos(meter->container, x, y);
    lv_obj_set_style_bg_color(meter->container, COLOR_BG, 0);
    lv_obj_set_style_border_width(meter->container, 0, 0);
    lv_obj_set_style_pad_all(meter->container, THEME_SPACING_XS, 0);
    lv_obj_set_flex_flow(meter->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(meter->container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(meter->container, THEME_SPACING_S, 0);
    
    // Label da etiqueta (IN/OUT)
    meter->label = lv_label_create(meter->container);
    lv_label_set_text(meter->label, label_text);
    lv_obj_set_style_text_font(meter->label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(meter->label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_min_width(meter->label, 28, 0);
    
    // Background da barra (track)
    meter->bar_bg = lv_obj_create(meter->container);
    lv_obj_set_size(meter->bar_bg, width - 70, height - 8);
    lv_obj_set_style_bg_color(meter->bar_bg, COLOR_DISABLED, 0);
    lv_obj_set_style_bg_opa(meter->bar_bg, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(meter->bar_bg, THEME_RADIUS_S, 0);
    lv_obj_set_style_border_width(meter->bar_bg, 0, 0);
    lv_obj_set_style_pad_all(meter->bar_bg, 0, 0);
    
    // Barra indicadora (fill)
    meter->bar = lv_obj_create(meter->bar_bg);
    lv_obj_set_size(meter->bar, 0, height - 8);
    lv_obj_set_style_bg_color(meter->bar, COLOR_METER_SAFE, 0);
    lv_obj_set_style_bg_opa(meter->bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(meter->bar, THEME_RADIUS_S, 0);
    lv_obj_set_style_border_width(meter->bar, 0, 0);
    lv_obj_align(meter->bar, LV_ALIGN_LEFT_MID, 0, 0);
    
    // Peak hold marker (pequeno retângulo no final da barra)
    meter->peak_marker = lv_obj_create(meter->bar_bg);
    lv_obj_set_size(meter->peak_marker, 3, height - 12);
    lv_obj_set_style_bg_color(meter->peak_marker, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_bg_opa(meter->peak_marker, LV_OPA_80, 0);
    lv_obj_set_style_border_width(meter->peak_marker, 0, 0);
    lv_obj_align(meter->peak_marker, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_flag(meter->peak_marker, LV_OBJ_FLAG_HIDDEN);
    
    // Label do valor em dB
    meter->db_label = lv_label_create(meter->container);
    lv_label_set_text(meter->db_label, "-60.0");
    lv_obj_set_style_text_font(meter->db_label, &lv_font_montserrat_11, 0);
    lv_obj_set_style_text_color(meter->db_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_min_width(meter->db_label, 42, 0);
    lv_obj_set_flex_grow(meter->db_label, 0);
    
    // Inicializa estado
    meter->current_db = -60.0f;
    meter->peak_db = -60.0f;
    meter->peak_hold_time = 0;
    
    return meter;
}

void vu_meter_update(VuMeter_t* meter, float db) {
    if (!meter || !meter->bar || !meter->bar_bg) return;
    
    // Clamp do valor
    if (db < -60.0f) db = -60.0f;
    if (db > 0.0f) db = 0.0f;
    
    meter->current_db = db;
    
    // Calcula largura em pixels
    int32_t bar_max_width = lv_obj_get_width(meter->bar_bg);
    int32_t fill_width = db_to_pixels(db, bar_max_width);
    lv_obj_set_width(meter->bar, fill_width);
    
    // Atualiza cor baseada no nível
    lv_color_t color = get_meter_color(db);
    lv_obj_set_style_bg_color(meter->bar, color, 0);
    
    // Peak hold logic
    if (db > meter->peak_db) {
        meter->peak_db = db;
        meter->peak_hold_time = lv_tick_get();
        lv_obj_clear_flag(meter->peak_marker, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Move peak marker para posição atual do peak
    int32_t peak_x = db_to_pixels(meter->peak_db, bar_max_width) - 3;
    lv_obj_set_x(meter->peak_marker, peak_x);
    
    // Decay do peak após 800ms sem novo pico
    uint32_t elapsed = lv_tick_elaps(meter->peak_hold_time);
    if (elapsed > 800 && db < meter->peak_db - 2.0f) {
        // Release controlado: peak cai gradualmente
        meter->peak_db -= 1.0f;
        if (meter->peak_db < db) meter->peak_db = db;
        meter->peak_hold_time = lv_tick_get();
        peak_x = db_to_pixels(meter->peak_db, bar_max_width) - 3;
        lv_obj_set_x(meter->peak_marker, peak_x);
        if (meter->peak_db <= -59.0f) {
            lv_obj_add_flag(meter->peak_marker, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    // Atualiza label com valor numérico
    char buf[8];
    snprintf(buf, sizeof(buf), "%.1f", db);
    lv_label_set_text(meter->db_label, buf);
}

void vu_meter_reset_peak(VuMeter_t* meter) {
    if (!meter) return;
    meter->peak_db = -60.0f;
    lv_obj_add_flag(meter->peak_marker, LV_OBJ_FLAG_HIDDEN);
}

void vu_meter_set_peak_hold(VuMeter_t* meter, uint32_t hold_ms) {
    if (!meter) return;
    // Reservado para configuração futura
    (void)hold_ms;
}
