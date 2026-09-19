#ifndef VOXLINK_UI_H
#define VOXLINK_UI_H

// Capability queries the UI may ask the VoxLink glue. Implemented by
// VoxLinkGlue.cpp on the device. Kept as plain declarations so UI translation
// units do not depend on the client/transport headers.
#include <cstdint>

// True when the P4 advertised this parameter, or when no CAPS snapshot has been
// received yet (offline local development). The Effect Editor uses this to skip
// controls the connected firmware does not expose.
bool voxlink_param_supported(uint16_t voxlinkId);

#endif // VOXLINK_UI_H
