#ifndef COMPACT_PARAM_SET_H
#define COMPACT_PARAM_SET_H

#include "ParameterValue.h"
#include <cstddef>
#include <cstdint>

// Fixed count of VoxP4 parameters defined by contract (71).
#ifndef VOXP4_PARAM_COUNT
#define VOXP4_PARAM_COUNT 71u
#endif

class CompactParamSet {
public:
    static constexpr size_t kMaxParams = VOXP4_PARAM_COUNT;

    CompactParamSet() {
        clearAll();
    }

    void clearAll() {
        mask_[0] = 0;
        mask_[1] = 0;
        for (size_t i = 0; i < kMaxParams; ++i) {
            values_[i] = ParameterValue();
        }
    }

    void clear(size_t index) {
        if (index >= kMaxParams) return;
        const size_t word = index / 64;
        const size_t bit = index % 64;
        mask_[word] &= ~(1ULL << bit);
        values_[index] = ParameterValue();
    }

    bool has(size_t index) const {
        if (index >= kMaxParams) return false;
        const size_t word = index / 64;
        const size_t bit = index % 64;
        return (mask_[word] & (1ULL << bit)) != 0;
    }

    void set(size_t index, const ParameterValue& val) {
        if (index >= kMaxParams) return;
        const size_t word = index / 64;
        const size_t bit = index % 64;
        mask_[word] |= (1ULL << bit);
        values_[index] = val;
    }

    ParameterValue get(size_t index) const {
        if (index >= kMaxParams) return ParameterValue();
        return values_[index];
    }

    const ParameterValue* getIfPresent(size_t index) const {
        if (!has(index)) return nullptr;
        return &values_[index];
    }

    size_t count() const {
        size_t c = 0;
        for (size_t i = 0; i < kMaxParams; ++i) {
            if (has(i)) ++c;
        }
        return c;
    }

    bool isEmpty() const {
        return mask_[0] == 0 && mask_[1] == 0;
    }

    void mergeFrom(const CompactParamSet& other) {
        for (size_t i = 0; i < kMaxParams; ++i) {
            if (other.has(i)) {
                set(i, other.get(i));
            }
        }
    }

    bool operator==(const CompactParamSet& other) const {
        if (mask_[0] != other.mask_[0] || mask_[1] != other.mask_[1]) {
            return false;
        }
        for (size_t i = 0; i < kMaxParams; ++i) {
            if (has(i)) {
                if (values_[i] != other.values_[i]) return false;
            }
        }
        return true;
    }

    bool operator!=(const CompactParamSet& other) const {
        return !(*this == other);
    }

private:
    uint64_t mask_[2];
    ParameterValue values_[kMaxParams];
};

#endif // COMPACT_PARAM_SET_H
