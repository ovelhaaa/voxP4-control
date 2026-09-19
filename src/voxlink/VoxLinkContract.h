#ifndef VOXLINK_CONTRACT_H
#define VOXLINK_CONTRACT_H

// Synchronized VoxLink contract constants.
//
// GENERATED / SYNCHRONIZED FROM:
//   voxP4/integration/VoxP4ParamIds.h       (parameter IDs + count)
//   voxP4/integration/voxlink_v1_vectors.h  (golden frame vectors)
//
// The files under src/voxlink/generated/ are a versioned copy of those
// artifacts. Do NOT edit them by hand. Re-sync with:
//
//   python scripts/sync_voxlink_schema.py --voxp4 <path-to-voxP4-checkout>
//   python scripts/sync_voxlink_schema.py --voxp4 <path> --check
//
// The controller keeps its own logical UiParamId. These VOXP4_PARAM_* values are
// the wire/ABI IDs for the future VoxLink transport (M6) and must never be used
// as UiParamId ordinals. The build does not require a voxP4 checkout.

#include "voxlink/generated/VoxP4ContractPin.h"
#include "voxlink/generated/VoxP4ParamIds.h"

#endif // VOXLINK_CONTRACT_H
