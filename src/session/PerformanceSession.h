#ifndef PERFORMANCE_SESSION_H
#define PERFORMANCE_SESSION_H

#include "ParameterDiff.h"
#include "ResolvedState.h"
#include "StateResolver.h"
#include "model/CompactParamSet.h"
#include "model/LibraryModel.h"
#include <string>
#include <vector>

enum class TransitionPolicy {
    ClearTemporaryEdits = 0,
    CarryTemporaryEdits = 1
};

class PerformanceSession {
public:
    explicit PerformanceSession(Library* library = nullptr);

    void setLibrary(Library* library);
    Library* getLibrary() const { return library_; }

    // Active identifiers
    const std::string& getActiveSetlistId() const { return activeSetlistId_; }
    int getActiveEntryIndex() const { return activeEntryIndex_; }
    const std::string& getActiveSceneId() const { return activeSceneId_; }
    const std::string& getActiveSubsceneId() const { return activeSubsceneId_; }

    // Active object pointers
    const Setlist* getActiveSetlist() const;
    const Scene* getActiveScene() const;
    const Subscene* getActiveSubscene() const;
    const Preset* getActiveBasePreset() const;

    // Transition policy
    TransitionPolicy getTransitionPolicy() const { return transitionPolicy_; }
    void setTransitionPolicy(TransitionPolicy policy) { transitionPolicy_ = policy; }

    // Dirty state
    bool isDirty() const { return !temporaryEdits_.isEmpty(); }
    const CompactParamSet& getTemporaryEdits() const { return temporaryEdits_; }

    // Current resolved state
    const ResolvedState& getResolvedState() const { return resolvedState_; }

    // Navigation operations (all return parameter deltas to send to VoxLink)
    std::vector<ParamDelta> selectSetlist(const std::string& setlistId);
    std::vector<ParamDelta> nextScene();
    std::vector<ParamDelta> previousScene();
    std::vector<ParamDelta> selectScene(const std::string& sceneId);
    std::vector<ParamDelta> nextSubscene();
    std::vector<ParamDelta> previousSubscene();
    std::vector<ParamDelta> selectSubscene(const std::string& subsceneId);

    // Quick edits
    std::vector<ParamDelta> applyTemporaryEdit(uint16_t wireId, const ParameterValue& value);
    std::vector<ParamDelta> applyTemporaryEditDense(size_t denseIndex, const ParameterValue& value);

    // Commit (Save) and Revert
    // Commit rule: if active Subscene exists -> commits to Subscene overrides.
    //              if no active Subscene -> commits to Scene overrides.
    //              Never commits to base Preset.
    bool commitTemporaryEdits();

    // Revert rule: clears temporary edits, re-resolves state, returns deltas.
    std::vector<ParamDelta> revertTemporaryEdits();

    // Re-resolve state from current pointers (e.g. after external library update)
    std::vector<ParamDelta> refreshState();

private:
    std::vector<ParamDelta> transitionTo(
        const std::string& setlistId,
        int entryIndex,
        const std::string& sceneId,
        const std::string& subsceneId
    );

    ResolvedState computeCurrentResolvedState() const;

    Library* library_;
    std::string activeSetlistId_;
    int activeEntryIndex_;
    std::string activeSceneId_;
    std::string activeSubsceneId_;

    CompactParamSet temporaryEdits_;
    TransitionPolicy transitionPolicy_;
    ResolvedState resolvedState_;
};

#endif // PERFORMANCE_SESSION_H
