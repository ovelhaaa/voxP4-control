#ifndef STATE_RESOLVER_H
#define STATE_RESOLVER_H

#include "ResolvedState.h"
#include "model/CompactParamSet.h"
#include "model/LibraryModel.h"

class StateResolver {
public:
    // Pure function to resolve 5-tier inheritance:
    // Defaults -> Preset -> Scene -> Subscene -> TemporaryEdits.
    // Any pointer can be nullptr (in which case that tier has no overrides).
    static ResolvedState resolve(
        const Preset* preset,
        const Scene* scene,
        const Subscene* subscene,
        const CompactParamSet* temporaryEdits = nullptr
    );

    // Overload accepting raw CompactParamSets directly
    static ResolvedState resolveRaw(
        const CompactParamSet* presetOverrides,
        const CompactParamSet* sceneOverrides,
        const CompactParamSet* subsceneOverrides,
        const CompactParamSet* temporaryEdits = nullptr
    );
};

#endif // STATE_RESOLVER_H
