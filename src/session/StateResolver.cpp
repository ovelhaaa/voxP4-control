#include "StateResolver.h"

ResolvedState StateResolver::resolve(
    const Preset* preset,
    const Scene* scene,
    const Subscene* subscene,
    const CompactParamSet* temporaryEdits
) {
    const CompactParamSet* pOverrides = preset ? &preset->overrides : nullptr;
    const CompactParamSet* sOverrides = scene ? &scene->overrides : nullptr;
    const CompactParamSet* subOverrides = subscene ? &subscene->overrides : nullptr;

    return resolveRaw(pOverrides, sOverrides, subOverrides, temporaryEdits);
}

ResolvedState StateResolver::resolveRaw(
    const CompactParamSet* presetOverrides,
    const CompactParamSet* sceneOverrides,
    const CompactParamSet* subsceneOverrides,
    const CompactParamSet* temporaryEdits
) {
    // 1. Defaults
    ResolvedState state;

    // 2. Preset overrides
    if (presetOverrides) {
        for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
            if (presetOverrides->has(i)) {
                state.set(i, presetOverrides->get(i));
            }
        }
    }

    // 3. Scene overrides
    if (sceneOverrides) {
        for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
            if (sceneOverrides->has(i)) {
                state.set(i, sceneOverrides->get(i));
            }
        }
    }

    // 4. Subscene overrides
    if (subsceneOverrides) {
        for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
            if (subsceneOverrides->has(i)) {
                state.set(i, subsceneOverrides->get(i));
            }
        }
    }

    // 5. Temporary performance edits
    if (temporaryEdits) {
        for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
            if (temporaryEdits->has(i)) {
                state.set(i, temporaryEdits->get(i));
            }
        }
    }

    return state;
}
