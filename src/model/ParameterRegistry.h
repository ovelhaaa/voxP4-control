#ifndef PARAMETER_REGISTRY_H
#define PARAMETER_REGISTRY_H

#include "ParameterValue.h"
#include <cstddef>
#include <cstdint>

#ifndef VOXP4_PARAM_COUNT
#define VOXP4_PARAM_COUNT 71u
#endif

struct ParamDescriptor {
    uint16_t wireId;
    uint8_t denseIndex;
    const char* key;           // e.g. "harmony.enable"
    const char* semanticName;  // e.g. "HarmonyEnable"
    const char* dspBinding;    // e.g. "PitchShiftEnabled"
    const char* display;       // e.g. "Harmony Enable"
    ParamType type;
    float minValue;
    float maxValue;
    float defaultValue;
};

class ParameterRegistry {
public:
    static constexpr size_t kParamCount = VOXP4_PARAM_COUNT;

    // Get descriptor by dense index (0..70). Returns nullptr if out of bounds.
    static const ParamDescriptor* getByIndex(size_t index);

    // Get descriptor by VoxLink wire ID (e.g. 0x0100). Returns nullptr if unknown.
    static const ParamDescriptor* getByWireId(uint16_t wireId);

    // Get descriptor by semantic or key name (case-insensitive, punctuation-tolerant).
    // Matches "HarmonyEnable", "harmony.enable", "PitchShiftEnabled", "MicroshiftLeftCents", etc.
    static const ParamDescriptor* getByName(const char* name);

    // Find dense index from wire ID. Returns -1 if unknown.
    static int wireIdToDenseIndex(uint16_t wireId);

    // Find wire ID from dense index. Returns 0 if out of bounds.
    static uint16_t denseIndexToWireId(size_t index);

    // Get default ParameterValue for a dense index.
    static ParameterValue getDefaultValue(size_t index);

    // Validate and clamp a ParameterValue according to descriptor rules.
    // Returns true if valid/clamped successfully, false if type or range is irrecoverable.
    static bool validateAndClamp(const ParamDescriptor& desc, ParameterValue& inOut);
};

#endif // PARAMETER_REGISTRY_H
