#ifndef FX_CHAIN_SCREEN_H
#define FX_CHAIN_SCREEN_H

#include <lvgl.h>

void fx_chain_screen_init(lv_obj_t* parent);
void fx_chain_update_effect_state(int effectId, bool enabled, const char* paramName, const char* paramValue);

#endif  // FX_CHAIN_SCREEN_H
