#pragma once
/**
 * VoxP4 CYD - VU Meter Widget
 * 
 * Widget horizontal para exibição de níveis de áudio (input/output)
 * Otimizado para 320x240 e performance com LVGL
 */

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_obj_t* container;
    lv_obj_t* bar;
    lv_obj_t* db_label;
    lv_obj_t* clip_led;
    float current_db;
    float peak_db;
    uint32_t peak_hold_time;
    uint32_t clip_hold_until;
} VuMeter_t;

/**
 * Cria um VU Meter horizontal
 * @param parent objeto pai
 * @param x posição X (ignorado se usar flex)
 * @param y posição Y (ignorado se usar flex)
 * @param height altura do meter
 * @param label_text texto da etiqueta (ex: "IN", "OUT") - pode ser NULL ou ""
 * @return ponteiro para estrutura VuMeter_t
 */
VuMeter_t* vu_meter_create(lv_obj_t* parent, int32_t x, int32_t y, 
                           int32_t height, const char* label_text);

/**
 * Retorna o container do meter para ajustes de layout
 * @param meter ponteiro para VuMeter_t
 * @return lv_obj_t* container
 */
lv_obj_t* vu_meter_get_container(VuMeter_t* meter);

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

#ifdef __cplusplus
} /*extern "C"*/
#endif
