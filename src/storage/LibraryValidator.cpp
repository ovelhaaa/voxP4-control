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
    if (library.formatVersion != kSupportedFormatVersion) {
        res.errors.push_back("Unsupported formatVersion: " + std::to_string(library.formatVersion) +
                             " (supported: " + std::to_string(kSupportedFormatVersion) + ")");
    }
    if (library.schemaVersion != kSupportedSchemaVersion) {
        res.errors.push_back("Unsupported schemaVersion: " + std::to_string(library.schemaVersion) +
                             " (supported: " + std::to_string(kSupportedSchemaVersion) + ")");
    }
    if (library.libraryId.empty()) {
        res.errors.push_back("Missing required field: libraryId");
    } else if (library.libraryId.length() > kMaxStringLength) {
        res.errors.push_back("libraryId exceeds length limit of " + std::to_string(kMaxStringLength));
    }
    if (library.name.empty()) {
        res.errors.push_back("Missing required field: name");
    } else if (library.name.length() > kMaxStringLength) {
        res.errors.push_back("name exceeds length limit of " + std::to_string(kMaxStringLength));
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
        } else if (p.id.length() > kMaxStringLength) {
            res.errors.push_back("Preset id exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + p.id);
        } else if (!presetIds.insert(p.id).second) {
            res.errors.push_back("Duplicate preset id: " + p.id);
        }
        if (p.name.empty()) {
            res.errors.push_back("Preset '" + p.id + "' has empty name");
        } else if (p.name.length() > kMaxStringLength) {
            res.errors.push_back("Preset name exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + p.name);
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
        } else if (s.id.length() > kMaxStringLength) {
            res.errors.push_back("Scene id exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + s.id);
        } else if (!sceneIds.insert(s.id).second) {
            res.errors.push_back("Duplicate scene id: " + s.id);
        }
        if (s.name.empty()) {
            res.errors.push_back("Scene '" + s.id + "' has empty name");
        } else if (s.name.length() > kMaxStringLength) {
            res.errors.push_back("Scene name exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + s.name);
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
            } else if (sub.id.length() > kMaxStringLength) {
                res.errors.push_back("Subscene id exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + sub.id);
            } else if (!subsceneIds.insert(sub.id).second) {
                res.errors.push_back("Scene '" + s.id + "' has duplicate subscene id: " + sub.id);
            }
            if (sub.name.empty()) {
                res.errors.push_back("Subscene '" + sub.id + "' has empty name");
            } else if (sub.name.length() > kMaxStringLength) {
                res.errors.push_back("Subscene name exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + sub.name);
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
        } else if (sl.id.length() > kMaxStringLength) {
            res.errors.push_back("Setlist id exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + sl.id);
        } else if (!setlistIds.insert(sl.id).second) {
            res.errors.push_back("Duplicate setlist id: " + sl.id);
        }
        if (sl.name.empty()) {
            res.errors.push_back("Setlist '" + sl.id + "' has empty name");
        } else if (sl.name.length() > kMaxStringLength) {
            res.errors.push_back("Setlist name exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + sl.name);
        }

        if (sl.entries.size() > kMaxEntriesPerSetlist) {
            res.errors.push_back("Setlist '" + sl.id + "' exceeds entry limit of " + std::to_string(kMaxEntriesPerSetlist));
        }

        for (size_t e = 0; e < sl.entries.size(); ++e) {
            const auto& entry = sl.entries[e];
            if (entry.id.empty()) {
                res.errors.push_back("Setlist '" + sl.id + "' entry " + std::to_string(e) + " has empty id");
            } else if (entry.id.length() > kMaxStringLength) {
                res.errors.push_back("Setlist entry id exceeds length limit of " + std::to_string(kMaxStringLength) + ": " + entry.id);
            }
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
