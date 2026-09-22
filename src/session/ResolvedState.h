#ifndef RESOLVED_STATE_H
#define RESOLVED_STATE_H

#include "model/ParameterRegistry.h"
#include "model/ParameterValue.h"
#include <cstddef>
#include <cstdint>

class ResolvedState {
public:
    static constexpr size_t kParamCount = VOXP4_PARAM_COUNT;

    ResolvedState() {
        // Initialize with default values from registry
        for (size_t i = 0; i < kParamCount; ++i) {
            values_[i] = ParameterRegistry::getDefaultValue(i);
        }
    }

    ParameterValue get(size_t denseIndex) const {
        if (denseIndex >= kParamCount) return ParameterValue();
        return values_[denseIndex];
    }

    ParameterValue getByWireId(uint16_t wireId) const {
        int idx = ParameterRegistry::wireIdToDenseIndex(wireId);
        if (idx < 0) return ParameterValue();
        return values_[idx];
    }

    void set(size_t denseIndex, const ParameterValue& val) {
        if (denseIndex >= kParamCount) return;
        values_[denseIndex] = val;
    }

    void setByWireId(uint16_t wireId, const ParameterValue& val) {
        int idx = ParameterRegistry::wireIdToDenseIndex(wireId);
        if (idx >= 0) {
            values_[idx] = val;
        }
    }

    bool operator==(const ResolvedState& other) const {
        for (size_t i = 0; i < kParamCount; ++i) {
            if (values_[i] != other.values_[i]) return false;
        }
        return true;
    }

    bool operator!=(const ResolvedState& other) const {
        return !(*this == other);
    }

private:
    ParameterValue values_[kParamCount];
};

#endif // RESOLVED_STATE_H
