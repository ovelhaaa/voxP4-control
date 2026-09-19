#ifndef FX_CHAIN_SCREEN_H
#define FX_CHAIN_SCREEN_H

#include <lvgl.h>

void fx_chain_screen_init(lv_obj_t* parent);

// Updates one module. `mainValue` is the dominant parameter readout and
// `metadata` is the small auxiliary caption (e.g. "+3rd" / "KEY AUTO").
void fx_chain_update_effect_state(int effectId, bool enabled,
                                  const char* mainValue, const char* metadata);

#endif  // FX_CHAIN_SCREEN_H
