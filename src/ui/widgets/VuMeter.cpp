/**
 * VoxP4 CYD - VU Meter Widget Implementation
 * Horizontal meter otimizado para 320x240
 */

#include "VuMeter.h"
#include "../UiTheme.h"
#include <stdio.h>

// Converte dB (-60 a 0) para posição da barra (0-100%)
static int32_t db_to_percent(float db) {
    if (db <= -60.0f) return 0;
    if (db >= 0.0f) return 100;
    // Mapeamento linear simples para performance
    float normalized = (db + 60.0f) / 60.0f;
    return (int32_t)(normalized * 100.0f);
}

// Seleciona cor baseada no nível de dB - zones do meter
static lv_color_t get_meter_color(float db) {
    if (db >= -3.0f) {
        return COLOR_METER_CLIP;       // Clip zone: -3 a 0dB
    } else if (db >= -12.0f) {
        return COLOR_METER_WARNING;    // Warning zone: -12 a -3dB
    } else {
        return COLOR_METER_SAFE;       // Safe zone: -60 a -12dB
    }
}

VuMeter_t* vu_meter_create(lv_obj_t* parent, int32_t x, int32_t y, 
                           int32_t height, const char* label_text) {
    VuMeter_t* meter = (VuMeter_t*)lv_mem_alloc(sizeof(VuMeter_t));
    if (!meter) return NULL;
    
    // Container principal - horizontal bar
    meter->container = lv_obj_create(parent);
    lv_obj_set_size(meter->container, 180, height);
    lv_obj_set_pos(meter->container, x, y);
    lv_obj_set_style_bg_color(meter->container, lv_color_make(0x1A, 0x1F, 0x2A), 0);
    lv_obj_set_style_border_width(meter->container, 0, 0);
    lv_obj_set_style_radius(meter->container, RADIUS_S, 0);
    lv_obj_set_style_pad_all(meter->container, 2, 0);
    
    // Barra horizontal - reduzida para reservar espaço do label dB à direita
    meter->bar = lv_bar_create(meter->container);
    lv_obj_set_size(meter->bar, 140, height - 4);
    lv_obj_align(meter->bar, LV_ALIGN_LEFT_MID, 0, 0);
    lv_bar_set_range(meter->bar, 0, 100);
    lv_bar_set_value(meter->bar, 0, LV_ANIM_OFF);
    
    // Estilo da barra - cor dinâmica baseada no nível
    lv_obj_set_style_bg_color(meter->bar, COLOR_METER_SAFE, LV_PART_INDICATOR);
    lv_obj_set_style_radius(meter->bar, RADIUS_S, LV_PART_INDICATOR);
    
    // Background da barra
    lv_obj_set_style_bg_color(meter->bar, lv_color_make(0x2A, 0x2F, 0x3D), 0);
    lv_obj_set_style_radius(meter->bar, RADIUS_S, 0);
    
    // Label do valor em dB à direita
    meter->db_label = lv_label_create(meter->container);
    lv_label_set_text(meter->db_label, "-60.0");
    lv_obj_align(meter->db_label, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_text_font(meter->db_label, FONT_TINY, 0);
    lv_obj_set_style_text_color(meter->db_label, COLOR_TEXT_SECONDARY, 0);
    
    // Inicializa estado
    meter->current_db = -60.0f;
    meter->peak_db = -60.0f;
    meter->peak_hold_time = 0;
    
    (void)x; (void)y; (void)label_text;  // Unused in horizontal mode
    
    return meter;
}

lv_obj_t* vu_meter_get_container(VuMeter_t* meter) {
    if (!meter) return NULL;
    return meter->container;
}

void vu_meter_update(VuMeter_t* meter, float db) {
    if (!meter) return;
    
    // Clamp do valor
    if (db < -60.0f) db = -60.0f;
    if (db > 0.0f) db = 0.0f;
    
    meter->current_db = db;
    
    // Atualiza posição da barra
    int32_t percent = db_to_percent(db);
    lv_bar_set_value(meter->bar, percent, LV_ANIM_OFF);
    
    // Atualiza cor baseada no nível
    lv_color_t color = get_meter_color(db);
    lv_obj_set_style_bg_color(meter->bar, color, LV_PART_INDICATOR);
    
    // Atualiza label com valor numérico
    char buf[8];
    snprintf(buf, sizeof(buf), "%.1f", db);
    lv_label_set_text(meter->db_label, buf);
}

void vu_meter_reset_peak(VuMeter_t* meter) {
    if (!meter) return;
    meter->peak_db = -60.0f;
}
