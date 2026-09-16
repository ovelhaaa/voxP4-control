#pragma once
/**
 * VoxP4 CYD - VU Meter Widget
 * 
 * Widget HORIZONTAL para exibição de níveis de áudio (input/output)
 * Otimizado para Performance Screen 320x240
 * - Fast attack, controlled release
 * - Peak hold visual
 * - Zones coloridas (safe/warning/clip)
 * - Sem animações empilhadas para telemetria 20-30Hz
 */

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_obj_t* container;      // Container principal (flex row)
    lv_obj_t* label;          // Label IN/OUT
    lv_obj_t* bar_bg;         // Background/track da barra
    lv_obj_t* bar;            // Barra indicadora (fill)
    lv_obj_t* peak_marker;    // Marcador de pico
    lv_obj_t* db_label;       // Valor numérico em dB
    int32_t width;
    int32_t height;
    float current_db;
    float peak_db;
    uint32_t peak_hold_time;
} VuMeter_t;

/**
 * Cria um VU Meter horizontal
 * @param parent objeto pai
 * @param x posição X
 * @param y posição Y
 * @param width largura total do widget
 * @param height altura do widget
 * @param label_text texto da etiqueta (ex: "IN", "OUT")
 * @return ponteiro para estrutura VuMeter_t
 */
VuMeter_t* vu_meter_create(lv_obj_t* parent, int32_t x, int32_t y, 
                           int32_t width, int32_t height, const char* label_text);

/**
 * Atualiza o valor do meter em dB
 * @param meter ponteiro para VuMeter_t
 * @param db valor em dB (-60.0f a 0.0f)
 */
void vu_meter_update(VuMeter_t* meter, float db);

/**
 * Reseta o indicador de pico
 * @param meter ponteiro para VuMeter_t
 */
void vu_meter_reset_peak(VuMeter_t* meter);

/**
 * Define o tempo de hold do pico (ms)
 * @param meter ponteiro para VuMeter_t
 * @param hold_ms tempo em milissegundos
 */
void vu_meter_set_peak_hold(VuMeter_t* meter, uint32_t hold_ms);

#ifdef __cplusplus
} /*extern "C"*/
#endif
