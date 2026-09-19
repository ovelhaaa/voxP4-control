#ifndef EFFECT_EDIT_SCREEN_H
#define EFFECT_EDIT_SCREEN_H

#include <lvgl.h>

void effect_edit_screen_init(lv_obj_t* parent);

// Public API for updating the screen content
void effect_edit_load_effect(int effectId);

// Sync the compact ON/OFF toggle with the authoritative effect state.
// No-op unless `effectId` is the effect currently open in the editor.
void effect_edit_set_enabled(int effectId, bool enabled);

#endif
