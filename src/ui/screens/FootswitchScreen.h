#ifndef FOOTSWITCH_SCREEN_H
#define FOOTSWITCH_SCREEN_H

#include <lvgl.h>

// Initialize the footswitch configuration screen (status + local editing).
void footswitch_screen_init(lv_obj_t* parent);

// Update functions
void footswitch_update_config(int fsIndex, uint8_t mode, const char* actionName);
void footswitch_update_state(int fsIndex, bool pressed);

// Fan out the full editable configuration for one footswitch. Used by the App
// after a local edit so the editor and the status rows stay in sync.
void footswitch_update_edit(int fsIndex, uint8_t mode, uint8_t press,
                            uint8_t release, uint8_t longPress,
                            uint8_t doublePress);

#endif  // FOOTSWITCH_SCREEN_H
