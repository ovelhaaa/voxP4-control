#ifndef FX_CHAIN_SCREEN_H
#define FX_CHAIN_SCREEN_H

#include <lvgl.h>
#include "ui/params/UiParamModel.h"

void fx_chain_screen_init(lv_obj_t* parent);

// Updates one module. `mainValue` is the dominant parameter readout and
// `metadata` is the small auxiliary caption (e.g. "+3rd" / "KEY AUTO").
void fx_chain_update_effect_state(UiEffectId effect, bool enabled,
                                  const char* mainValue, const char* metadata);

// Reflects the local global-bypass flag on the BYPASS action.
void fx_chain_set_bypass(bool active);
// Global bypass has no VoxLink v1 backend; disable the action while connected.
void fx_chain_set_bypass_available(bool available);

#endif  // FX_CHAIN_SCREEN_H
