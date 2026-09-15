#pragma once
/**
 * VoxP4 CYD - VU Meter Widget
 * 
 * Widget vertical para exibição de níveis de áudio (input/output)
 * Otimizado para performance com LVGL
 */

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Cores do meter por faixa
#define METER_COLOR_GREEN   LV_COLOR_MAKE(0x00, 0xE6, 0x76)  // -60dB a -12dB
#define METER_COLOR_YELLOW  LV_COLOR_MAKE(0xFF, 0xD7, 0x40)  // -12dB a -3dB
#define METER_COLOR_RED     LV_COLOR_MAKE(0xFF, 0x52, 0x52)  // -3dB a 0dB

// Dimensões padrão
#define METER_DEFAULT_WIDTH     40
#define METER_DEFAULT_HEIGHT    120
#define METER_BAR_WIDTH         28

typedef struct {
    lv_obj_t* container;
    lv_obj_t* bar;
    lv_obj_t* peak_indicator;
    lv_obj_t* db_label;
    float current_db;
    float peak_db;
    uint32_t peak_hold_time;
} VuMeter_t;

/**
 * Inicializa um VU Meter vertical
 * @param parent objeto pai
 * @param x posição X
 * @param y posição Y
 * @param height altura do meter
 * @param label_text texto da etiqueta (ex: "IN", "OUT")
 * @return ponteiro para estrutura VuMeter_t
 */
VuMeter_t* vu_meter_create(lv_obj_t* parent, int32_t x, int32_t y, 
                           int32_t height, const char* label_text);

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
