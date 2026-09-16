#pragma once
/**
 * VoxP4 CYD - Effect Card Widget
 * 
 * Card para exibição de efeito na chain ou performance screen
 * Mostra nome, status (ON/OFF) e parâmetro principal
 */

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EFFECT_HARMONY,
    EFFECT_REVERB,
    EFFECT_DELAY,
    EFFECT_LIMITER
} EffectType_t;

typedef struct {
    lv_obj_t* card;
    lv_obj_t* name_label;
    lv_obj_t* status_indicator;
    lv_obj_t* param_label;
    EffectType_t effect_type;
    bool is_enabled;
} EffectCard_t;

/**
 * Cria um Effect Card
 * @param parent objeto pai
 * @param x posição X
 * @param y posição Y
 * @param effect_type tipo do efeito
 * @return ponteiro para EffectCard_t
 */
EffectCard_t* effect_card_create(lv_obj_t* parent, int32_t x, int32_t y, 
                                  EffectType_t effect_type);

/**
 * Atualiza estado ON/OFF do efeito
 * @param card ponteiro para EffectCard_t
 * @param enabled true = ON, false = OFF
 */
void effect_card_set_enabled(EffectCard_t* card, bool enabled);

/**
 * Atualiza texto do parâmetro principal
 * @param card ponteiro para EffectCard_t
 * @param param_text texto do parâmetro (ex: "+3rd", "2.1s")
 */
void effect_card_set_param(EffectCard_t* card, const char* param_text);

/**
 * Retorna o tipo do efeito
 * @param card ponteiro para EffectCard_t
 * @return EffectType_t
 */
EffectType_t effect_card_get_type(EffectCard_t* card);

/**
 * Retorna se o efeito está habilitado
 * @param card ponteiro para EffectCard_t
 * @return true se enabled
 */
bool effect_card_is_enabled(EffectCard_t* card);

#ifdef __cplusplus
} /*extern "C"*/
#endif
