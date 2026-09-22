#include "ParameterDiff.h"

std::vector<ParamDelta> ParameterDiff::compute(
    const ResolvedState& oldState,
    const ResolvedState& newState
) {
    std::vector<ParamDelta> deltas;
    deltas.reserve(16); // small initial reservation, typical transitions change 1-8 params

    for (size_t i = 0; i < ResolvedState::kParamCount; ++i) {
        ParameterValue oldVal = oldState.get(i);
        ParameterValue newVal = newState.get(i);

        if (oldVal != newVal) {
            const ParamDescriptor* desc = ParameterRegistry::getByIndex(i);
            ParamDelta delta;
            delta.denseIndex = static_cast<uint8_t>(i);
            delta.wireId = desc ? desc->wireId : ParameterRegistry::denseIndexToWireId(i);
            delta.value = newVal;
            delta.descriptor = desc;
            deltas.push_back(delta);
        }
    }

    return deltas;
}
