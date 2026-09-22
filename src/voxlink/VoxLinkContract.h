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
// The controller uses these VOXP4_PARAM_* values as the canonical parameter
// keys for its UI state (see src/model/ParameterRegistry.h and
// src/ui/params/UiParamState.h). They must never be treated as array indices;
// the dense index comes from ParameterRegistry. The build does not require a
// voxP4 checkout.

#include "voxlink/generated/VoxP4ContractPin.h"
#include "voxlink/generated/VoxP4ParamIds.h"

#endif // VOXLINK_CONTRACT_H
