#ifndef LIBRARY_MODEL_H
#define LIBRARY_MODEL_H

#include "CompactParamSet.h"
#include <string>
#include <vector>

struct Preset {
    std::string id;
    std::string name;
    CompactParamSet overrides;
};

struct Subscene {
    std::string id;
    std::string name;
    CompactParamSet overrides;
};

struct SceneMetadata {
    std::string artist;
    std::string notes;
    std::string tags;
};

struct Scene {
    std::string id;
    std::string name;
    std::string basePresetId;
    SceneMetadata metadata;
    CompactParamSet overrides;
    std::vector<Subscene> subscenes;

    const Subscene* findSubscene(const std::string& subsceneId) const {
        for (const auto& s : subscenes) {
            if (s.id == subsceneId) return &s;
        }
        return nullptr;
    }

    Subscene* findSubsceneMut(const std::string& subsceneId) {
        for (auto& s : subscenes) {
            if (s.id == subsceneId) return &s;
        }
        return nullptr;
    }
};

struct SetlistEntry {
    std::string id;
    std::string sceneId;
};

struct Setlist {
    std::string id;
    std::string name;
    std::vector<SetlistEntry> entries;
};

struct Library {
    std::string format;
    uint32_t formatVersion;
    uint32_t schemaVersion;
    std::string libraryId;
    std::string name;

    std::vector<Preset> presets;
    std::vector<Scene> scenes;
    std::vector<Setlist> setlists;

    Library() : format("voxp4-library"), formatVersion(1), schemaVersion(1) {}

    const Preset* findPreset(const std::string& presetId) const {
        for (const auto& p : presets) {
            if (p.id == presetId) return &p;
        }
        return nullptr;
    }

    Preset* findPresetMut(const std::string& presetId) {
        for (auto& p : presets) {
            if (p.id == presetId) return &p;
        }
        return nullptr;
    }

    const Scene* findScene(const std::string& sceneId) const {
        for (const auto& s : scenes) {
            if (s.id == sceneId) return &s;
        }
        return nullptr;
    }

    Scene* findSceneMut(const std::string& sceneId) {
        for (auto& s : scenes) {
            if (s.id == sceneId) return &s;
        }
        return nullptr;
    }

    const Setlist* findSetlist(const std::string& setlistId) const {
        for (const auto& s : setlists) {
            if (s.id == setlistId) return &s;
        }
        return nullptr;
    }

    Setlist* findSetlistMut(const std::string& setlistId) {
        for (auto& s : setlists) {
            if (s.id == setlistId) return &s;
        }
        return nullptr;
    }
};

#endif // LIBRARY_MODEL_H
