#ifndef VOXLINK_GLUE_H
#define VOXLINK_GLUE_H

// Device-side VoxLink client glue. voxlink_glue_init() starts the UART task;
// voxlink_glue_tick() must be called from the UI context (main loop) and is the
// only place that applies VoxLink events to the UI.
void voxlink_glue_init();
void voxlink_glue_tick();

#endif // VOXLINK_GLUE_H
