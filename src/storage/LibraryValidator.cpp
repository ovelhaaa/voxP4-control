#include "LibraryValidator.h"
#include "model/ParameterRegistry.h"
#include <cmath>
#include <set>

ValidationResult LibraryValidator::validate(const Library& library) {
    ValidationResult res;

    // 1. Format and version
    if (library.format != "voxp4-library") {
        res.errors.push_back("Invalid format: expected 'voxp4-library', got '" + library.format + "'");
    }
    if (library.formatVersion > kSupportedFormatVersion) {
        res.errors.push_back("Unsupported formatVersion: " + std::to_string(library.formatVersion) +
                             " (supported: <= " + std::to_string(kSupportedFormatVersion) + ")");
    }
    if (library.formatVersion == 0) {
        res.errors.push_back("Invalid formatVersion: 0");
    }
    if (library.libraryId.empty()) {
        res.errors.push_back("Missing required field: libraryId");
    }

    // 2. Collection limits
    if (library.presets.size() > kMaxPresets) {
        res.errors.push_back("Preset count exceeds limit of " + std::to_string(kMaxPresets));
    }
    if (library.scenes.size() > kMaxScenes) {
        res.errors.push_back("Scene count exceeds limit of " + std::to_string(kMaxScenes));
    }
    if (library.setlists.size() > kMaxSetlists) {
        res.errors.push_back("Setlist count exceeds limit of " + std::to_string(kMaxSetlists));
    }

    // 3. Duplicate IDs & Preset validation
    std::set<std::string> presetIds;
    for (const auto& p : library.presets) {
        if (p.id.empty()) {
            res.errors.push_back("Preset has empty id");
        } else if (!presetIds.insert(p.id).second) {
            res.errors.push_back("Duplicate preset id: " + p.id);
        }
        if (p.name.length() > kMaxStringLength) {
            res.warnings.push_back("Preset name exceeds suggested length: " + p.name);
        }

        // Validate preset parameter overrides
        for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
            if (p.overrides.has(i)) {
                const ParamDescriptor* desc = ParameterRegistry::getByIndex(i);
                if (!desc) {
                    res.errors.push_back("Preset '" + p.id + "' references invalid parameter index: " + std::to_string(i));
                } else {
                    float val = p.overrides.get(i).asFloat();
                    if (std::isnan(val)) {
                        res.errors.push_back("Preset '" + p.id + "' has NaN value for parameter: " + desc->semanticName);
                    } else if (val < desc->minValue || val > desc->maxValue) {
                        res.errors.push_back("Preset '" + p.id + "' parameter '" + desc->semanticName +
                                             "' out of range: " + std::to_string(val));
                    }
                }
            }
        }
    }

    // 4. Duplicate IDs & Scene validation + Referential integrity
    std::set<std::string> sceneIds;
    for (const auto& s : library.scenes) {
        if (s.id.empty()) {
            res.errors.push_back("Scene has empty id");
        } else if (!sceneIds.insert(s.id).second) {
            res.errors.push_back("Duplicate scene id: " + s.id);
        }

        // Referential integrity: basePresetId
        if (!s.basePresetId.empty() && presetIds.find(s.basePresetId) == presetIds.end()) {
            res.errors.push_back("Scene '" + s.id + "' references missing basePresetId: " + s.basePresetId);
        }

        // Scene overrides
        for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
            if (s.overrides.has(i)) {
                const ParamDescriptor* desc = ParameterRegistry::getByIndex(i);
                if (!desc) {
                    res.errors.push_back("Scene '" + s.id + "' references invalid parameter index: " + std::to_string(i));
                } else {
                    float val = s.overrides.get(i).asFloat();
                    if (std::isnan(val)) {
                        res.errors.push_back("Scene '" + s.id + "' has NaN value for parameter: " + desc->semanticName);
                    } else if (val < desc->minValue || val > desc->maxValue) {
                        res.errors.push_back("Scene '" + s.id + "' parameter '" + desc->semanticName +
                                             "' out of range: " + std::to_string(val));
                    }
                }
            }
        }

        // Subscenes
        if (s.subscenes.size() > kMaxSubscenesPerScene) {
            res.errors.push_back("Scene '" + s.id + "' exceeds subscene limit of " + std::to_string(kMaxSubscenesPerScene));
        }
        std::set<std::string> subsceneIds;
        for (const auto& sub : s.subscenes) {
            if (sub.id.empty()) {
                res.errors.push_back("Scene '" + s.id + "' has subscene with empty id");
            } else if (!subsceneIds.insert(sub.id).second) {
                res.errors.push_back("Scene '" + s.id + "' has duplicate subscene id: " + sub.id);
            }

            for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
                if (sub.overrides.has(i)) {
                    const ParamDescriptor* desc = ParameterRegistry::getByIndex(i);
                    if (!desc) {
                        res.errors.push_back("Subscene '" + sub.id + "' references invalid parameter index: " + std::to_string(i));
                    } else {
                        float val = sub.overrides.get(i).asFloat();
                        if (std::isnan(val)) {
                            res.errors.push_back("Subscene '" + sub.id + "' has NaN value for parameter: " + desc->semanticName);
                        } else if (val < desc->minValue || val > desc->maxValue) {
                            res.errors.push_back("Subscene '" + sub.id + "' parameter '" + desc->semanticName +
                                                 "' out of range: " + std::to_string(val));
                        }
                    }
                }
            }
        }
    }

    // 5. Duplicate IDs & Setlist validation + Referential integrity
    std::set<std::string> setlistIds;
    for (const auto& sl : library.setlists) {
        if (sl.id.empty()) {
            res.errors.push_back("Setlist has empty id");
        } else if (!setlistIds.insert(sl.id).second) {
            res.errors.push_back("Duplicate setlist id: " + sl.id);
        }

        if (sl.entries.size() > kMaxEntriesPerSetlist) {
            res.errors.push_back("Setlist '" + sl.id + "' exceeds entry limit of " + std::to_string(kMaxEntriesPerSetlist));
        }

        for (size_t e = 0; e < sl.entries.size(); ++e) {
            const auto& entry = sl.entries[e];
            if (entry.sceneId.empty()) {
                res.errors.push_back("Setlist '" + sl.id + "' entry " + std::to_string(e) + " has empty sceneId");
            } else if (sceneIds.find(entry.sceneId) == sceneIds.end()) {
                res.errors.push_back("Setlist '" + sl.id + "' entry " + std::to_string(e) +
                                     " references missing sceneId: " + entry.sceneId);
            }
        }
    }

    return res;
}
