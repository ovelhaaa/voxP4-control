#include "PerformanceSession.h"

PerformanceSession::PerformanceSession(Library* library)
    : library_(nullptr),
      activeEntryIndex_(-1),
      transitionPolicy_(TransitionPolicy::ClearTemporaryEdits) {
    if (library) {
        setLibrary(library);
    }
}

void PerformanceSession::setLibrary(Library* library) {
    library_ = library;
    activeSetlistId_.clear();
    activeEntryIndex_ = -1;
    activeSceneId_.clear();
    activeSubsceneId_.clear();
    temporaryEdits_.clearAll();

    if (library_ && !library_->setlists.empty()) {
        selectSetlist(library_->setlists[0].id);
    } else if (library_ && !library_->scenes.empty()) {
        selectScene(library_->scenes[0].id);
    } else {
        refreshState();
    }
}

const Setlist* PerformanceSession::getActiveSetlist() const {
    if (!library_ || activeSetlistId_.empty()) return nullptr;
    return library_->findSetlist(activeSetlistId_);
}

const Scene* PerformanceSession::getActiveScene() const {
    if (!library_ || activeSceneId_.empty()) return nullptr;
    return library_->findScene(activeSceneId_);
}

const Subscene* PerformanceSession::getActiveSubscene() const {
    const Scene* sc = getActiveScene();
    if (!sc || activeSubsceneId_.empty()) return nullptr;
    return sc->findSubscene(activeSubsceneId_);
}

const Preset* PerformanceSession::getActiveBasePreset() const {
    const Scene* sc = getActiveScene();
    if (!sc || !library_ || sc->basePresetId.empty()) return nullptr;
    return library_->findPreset(sc->basePresetId);
}

ResolvedState PerformanceSession::computeCurrentResolvedState() const {
    const Preset* preset = getActiveBasePreset();
    const Scene* scene = getActiveScene();
    const Subscene* subscene = getActiveSubscene();

    return StateResolver::resolve(preset, scene, subscene, &temporaryEdits_);
}

std::vector<ParamDelta> PerformanceSession::refreshState() {
    ResolvedState newState = computeCurrentResolvedState();
    std::vector<ParamDelta> deltas = ParameterDiff::compute(resolvedState_, newState);
    resolvedState_ = newState;
    return deltas;
}

std::vector<ParamDelta> PerformanceSession::transitionTo(
    const std::string& setlistId,
    int entryIndex,
    const std::string& sceneId,
    const std::string& subsceneId
) {
    if (transitionPolicy_ == TransitionPolicy::ClearTemporaryEdits) {
        temporaryEdits_.clearAll();
    }

    activeSetlistId_ = setlistId;
    activeEntryIndex_ = entryIndex;
    activeSceneId_ = sceneId;
    activeSubsceneId_ = subsceneId;

    return refreshState();
}

std::vector<ParamDelta> PerformanceSession::selectSetlist(const std::string& setlistId) {
    if (!library_) return {};
    const Setlist* sl = library_->findSetlist(setlistId);
    if (!sl) return {};

    if (sl->entries.empty()) {
        return transitionTo(sl->id, -1, "", "");
    }

    const SetlistEntry& entry = sl->entries[0];
    const Scene* sc = library_->findScene(entry.sceneId);
    std::string firstSubscene = (sc && !sc->subscenes.empty()) ? sc->subscenes[0].id : "";
    return transitionTo(sl->id, 0, entry.sceneId, firstSubscene);
}

std::vector<ParamDelta> PerformanceSession::selectScene(const std::string& sceneId) {
    if (!library_) return {};
    const Scene* sc = library_->findScene(sceneId);
    if (!sc) return {};

    // Check if we are in a setlist that references this scene
    int entryIdx = -1;
    const Setlist* sl = getActiveSetlist();
    if (sl) {
        for (size_t i = 0; i < sl->entries.size(); ++i) {
            if (sl->entries[i].sceneId == sceneId) {
                entryIdx = static_cast<int>(i);
                break;
            }
        }
    }

    std::string firstSubscene = (!sc->subscenes.empty()) ? sc->subscenes[0].id : "";
    return transitionTo(sl ? sl->id : "", entryIdx, sc->id, firstSubscene);
}

std::vector<ParamDelta> PerformanceSession::nextScene() {
    const Setlist* sl = getActiveSetlist();
    if (sl && !sl->entries.empty()) {
        // Navigate inside active Setlist
        if (activeEntryIndex_ + 1 < static_cast<int>(sl->entries.size())) {
            int newIdx = activeEntryIndex_ + 1;
            const std::string& nextSceneId = sl->entries[newIdx].sceneId;
            const Scene* sc = library_ ? library_->findScene(nextSceneId) : nullptr;
            std::string firstSub = (sc && !sc->subscenes.empty()) ? sc->subscenes[0].id : "";
            return transitionTo(sl->id, newIdx, nextSceneId, firstSub);
        }
        // At end of setlist: no-wrap during performance
        return {};
    }

    // No active setlist: navigate library scenes
    if (library_ && !library_->scenes.empty()) {
        for (size_t i = 0; i < library_->scenes.size(); ++i) {
            if (library_->scenes[i].id == activeSceneId_) {
                if (i + 1 < library_->scenes.size()) {
                    const Scene& nextSc = library_->scenes[i + 1];
                    std::string firstSub = (!nextSc.subscenes.empty()) ? nextSc.subscenes[0].id : "";
                    return transitionTo("", -1, nextSc.id, firstSub);
                }
                break;
            }
        }
    }
    return {};
}

std::vector<ParamDelta> PerformanceSession::previousScene() {
    const Setlist* sl = getActiveSetlist();
    if (sl && !sl->entries.empty()) {
        if (activeEntryIndex_ > 0) {
            int newIdx = activeEntryIndex_ - 1;
            const std::string& prevSceneId = sl->entries[newIdx].sceneId;
            const Scene* sc = library_ ? library_->findScene(prevSceneId) : nullptr;
            std::string firstSub = (sc && !sc->subscenes.empty()) ? sc->subscenes[0].id : "";
            return transitionTo(sl->id, newIdx, prevSceneId, firstSub);
        }
        // At start of setlist: no-wrap
        return {};
    }

    if (library_ && !library_->scenes.empty()) {
        for (size_t i = 0; i < library_->scenes.size(); ++i) {
            if (library_->scenes[i].id == activeSceneId_) {
                if (i > 0) {
                    const Scene& prevSc = library_->scenes[i - 1];
                    std::string firstSub = (!prevSc.subscenes.empty()) ? prevSc.subscenes[0].id : "";
                    return transitionTo("", -1, prevSc.id, firstSub);
                }
                break;
            }
        }
    }
    return {};
}

std::vector<ParamDelta> PerformanceSession::selectSubscene(const std::string& subsceneId) {
    const Scene* sc = getActiveScene();
    if (!sc) return {};
    const Subscene* sub = sc->findSubscene(subsceneId);
    if (!sub) return {};

    return transitionTo(activeSetlistId_, activeEntryIndex_, activeSceneId_, sub->id);
}

std::vector<ParamDelta> PerformanceSession::nextSubscene() {
    const Scene* sc = getActiveScene();
    if (!sc || sc->subscenes.empty()) return {};

    for (size_t i = 0; i < sc->subscenes.size(); ++i) {
        if (sc->subscenes[i].id == activeSubsceneId_) {
            if (i + 1 < sc->subscenes.size()) {
                return transitionTo(activeSetlistId_, activeEntryIndex_, activeSceneId_, sc->subscenes[i + 1].id);
            }
            // At last subscene: no-wrap during live performance
            return {};
        }
    }

    // If no active subscene, select first
    return selectSubscene(sc->subscenes[0].id);
}

std::vector<ParamDelta> PerformanceSession::previousSubscene() {
    const Scene* sc = getActiveScene();
    if (!sc || sc->subscenes.empty()) return {};

    for (size_t i = 0; i < sc->subscenes.size(); ++i) {
        if (sc->subscenes[i].id == activeSubsceneId_) {
            if (i > 0) {
                return transitionTo(activeSetlistId_, activeEntryIndex_, activeSceneId_, sc->subscenes[i - 1].id);
            }
            // At first subscene: no-wrap
            return {};
        }
    }

    return selectSubscene(sc->subscenes[0].id);
}

std::vector<ParamDelta> PerformanceSession::applyTemporaryEdit(uint16_t wireId, const ParameterValue& value) {
    int idx = ParameterRegistry::wireIdToDenseIndex(wireId);
    if (idx < 0) return {};
    return applyTemporaryEditDense(static_cast<size_t>(idx), value);
}

std::vector<ParamDelta> PerformanceSession::applyTemporaryEditDense(size_t denseIndex, const ParameterValue& value) {
    if (denseIndex >= CompactParamSet::kMaxParams) return {};
    const ParamDescriptor* desc = ParameterRegistry::getByIndex(denseIndex);
    if (!desc) return {};

    ParameterValue clamped = value;
    ParameterRegistry::validateAndClamp(*desc, clamped);

    temporaryEdits_.set(denseIndex, clamped);
    return refreshState();
}

bool PerformanceSession::commitTemporaryEdits() {
    if (temporaryEdits_.isEmpty() || !library_) return false;

    Scene* sc = library_->findSceneMut(activeSceneId_);
    if (!sc) return false;

    if (!activeSubsceneId_.empty()) {
        Subscene* sub = sc->findSubsceneMut(activeSubsceneId_);
        if (sub) {
            sub->overrides.mergeFrom(temporaryEdits_);
            temporaryEdits_.clearAll();
            return true;
        }
    }

    // Fallback if no subscene exists: commit to Scene overrides
    sc->overrides.mergeFrom(temporaryEdits_);
    temporaryEdits_.clearAll();
    return true;
}

std::vector<ParamDelta> PerformanceSession::revertTemporaryEdits() {
    if (temporaryEdits_.isEmpty()) return {};
    temporaryEdits_.clearAll();
    return refreshState();
}
