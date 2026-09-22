#ifndef PARAMETER_DIFF_H
#define PARAMETER_DIFF_H

#include "ResolvedState.h"
#include "model/ParameterRegistry.h"
#include <cstdint>
#include <vector>

struct ParamDelta {
    uint16_t wireId;
    uint8_t denseIndex;
    ParameterValue value;
    const ParamDescriptor* descriptor;
};

class ParameterDiff {
public:
    // Compute deltas between old and new state.
    // Returns list of changed parameters only.
    static std::vector<ParamDelta> compute(
        const ResolvedState& oldState,
        const ResolvedState& newState
    );
};

#endif // PARAMETER_DIFF_H
