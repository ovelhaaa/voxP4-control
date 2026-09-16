/**
 * VoxP4 CYD - VU Meter Widget Implementation
 */

#include "VuMeter.h"
#include "../UiTheme.h"
#include <stdio.h>

// Converte dB (-60 a 0) para posição da barra (0-100%)
static int32_t db_to_percent(float db) {
    if (db <= -60.0f) return 0;
    if (db >= 0.0f) return 100;
    // Mapeamento não-linear: mais resolução perto de 0dB
    float normalized = (db + 60.0f) / 60.0f;  // 0.0 a 1.0
    return (int32_t)(normalized * 100.0f);
}

// Seleciona cor baseada no nível de dB - zones do meter
static lv_color_t get_meter_color(float db) {
    if (db >= -3.0f) {
        return METER_COLOR_RED;      // Clip zone
    } else if (db >= -12.0f) {
        return METER_COLOR_YELLOW;   // Warning zone
    } else {
        return METER_COLOR_GREEN;    // Safe zone
    }
}

VuMeter_t* vu_meter_create(lv_obj_t* parent, int32_t x, int32_t y, 
                           int32_t height, const char* label_text) {
    VuMeter_t* meter = (VuMeter_t*)lv_malloc(sizeof(VuMeter_t));
    if (!meter) return NULL;
    
    // Container principal
    meter->container = lv_obj_create(parent);
    lv_obj_set_size(meter->container, METER_DEFAULT_WIDTH, height);
    lv_obj_set_pos(meter->container, x, y);
    lv_obj_set_style_bg_color(meter->container, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(meter->container, 0, 0);
    lv_obj_set_style_pad_all(meter->container, 6, 0);
    lv_obj_set_flex_grow(meter->container, 0);
    
    // Barra vertical (LVGL bar é horizontal por padrão, usamos mode para vertical)
    meter->bar = lv_bar_create(meter->container);
    lv_obj_set_size(meter->bar, METER_BAR_WIDTH, height - 40);
    lv_obj_align(meter->bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_bar_set_range(meter->bar, 0, 100);
    lv_bar_set_value(meter->bar, 0, LV_ANIM_OFF);
    lv_bar_set_mode(meter->bar, LV_BAR_MODE_NORMAL);
    
    // Estilo da barra - gradiente baseado no nível
    lv_obj_set_style_bg_color(meter->bar, METER_COLOR_GREEN, LV_PART_INDICATOR);
    
    // Indicador de pico (linha horizontal que marca o pico atingido)
    meter->peak_indicator = lv_obj_create(meter->container);
    lv_obj_set_size(meter->peak_indicator, METER_BAR_WIDTH + 4, 3);
    lv_obj_align(meter->peak_indicator, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(meter->peak_indicator, COLOR_METER_PEAK, 0);
    lv_obj_set_style_border_width(meter->peak_indicator, 0, 0);
    lv_obj_add_flag(meter->peak_indicator, LV_OBJ_FLAG_HIDDEN);
    
    // Label do valor em dB
    meter->db_label = lv_label_create(meter->container);
    lv_label_set_text(meter->db_label, "--");
    lv_obj_align_to(meter->db_label, meter->bar, LV_ALIGN_BOTTOM_MID, 0, 8);
    lv_obj_set_style_text_font(meter->db_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(meter->db_label, COLOR_TEXT_SECONDARY, 0);
    
    // Label da etiqueta (IN/OUT)
    lv_obj_t* type_label = lv_label_create(meter->container);
    lv_label_set_text(type_label, label_text);
    lv_obj_align(type_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_font(type_label, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(type_label, COLOR_TEXT_MUTED, 0);
    
    // Inicializa estado
    meter->current_db = -60.0f;
    meter->peak_db = -60.0f;
    meter->peak_hold_time = 0;
    meter->height = height;
    
    return meter;
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
    
    // Atualiza indicador de pico se necessário
    if (db > meter->peak_db) {
        meter->peak_db = db;
        meter->peak_hold_time = lv_tick_get();
        
        // Move peak indicator para nova posição
        int32_t peak_y = (meter->height - 40) - ((meter->height - 40) * percent / 100);
        lv_obj_set_y(meter->peak_indicator, peak_y);
        lv_obj_clear_flag(meter->peak_indicator, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Timeout do peak hold (500ms)
    if (lv_tick_elaps(meter->peak_hold_time) > 500) {
        meter->peak_db = db;  // Reset para valor atual
        meter->peak_hold_time = lv_tick_get();
        int32_t peak_y = (meter->height - 40) - ((meter->height - 40) * percent / 100);
        lv_obj_set_y(meter->peak_indicator, peak_y);
    }
    
    // Atualiza label com valor numérico
    char buf[8];
    snprintf(buf, sizeof(buf), "%.1f", db);
    lv_label_set_text(meter->db_label, buf);
}

void vu_meter_reset_peak(VuMeter_t* meter) {
    if (!meter) return;
    meter->peak_db = -60.0f;
    lv_obj_add_flag(meter->peak_indicator, LV_OBJ_FLAG_HIDDEN);
}

void vu_meter_set_peak_hold(VuMeter_t* meter, uint32_t hold_ms) {
    if (!meter) return;
    // Implementação futura para customização do hold time
    (void)hold_ms;
}
