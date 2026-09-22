#include "LibrarySerializer.h"
#include "model/ParameterRegistry.h"
#include <ArduinoJson.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>

static void toLowerString(const char* src, char* dst, size_t dstSize) {
    size_t d = 0;
    for (size_t s = 0; src[s] != '\0' && d + 1 < dstSize; ++s) {
        char c = src[s];
        if (c == '.' || c == '_' || c == '-' || c == ' ') continue;
        dst[d++] = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    dst[d] = '\0';
}

// Canonical enum string tables matching VoxP4 DSP source definitions
static const char* const kHarmonyModeStrings[] = { "Fixed", "Diatonic", "Midi" };

static const char* const kHarmonyKeyStrings[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static const char* const kHarmonyScaleStrings[] = {
    "Major", "NaturalMinor", "HarmonicMinor", "MelodicMinor",
    "Dorian", "Phrygian", "Lydian", "Mixolydian",
    "Locrian", "MajorPentatonic", "MinorPentatonic", "BluesMinor"
};

static const char* const kNonScalePolicyStrings[] = { "Nearest", "Chromatic", "Bypass" };

static const char* const kTempoSubdivisionStrings[] = {
    "Whole", "Half", "Quarter", "Eighth", "Sixteenth", "ThirtySecond",
    "DottedHalf", "DottedQuarter", "DottedEighth", "DottedSixteenth",
    "TripletQuarter", "TripletEighth", "TripletSixteenth"
};

static const char* const kSpatialRoutingStrings[] = { "Parallel", "DelayIntoReverb" };

static const char* const kSpatialSourceStrings[] = { "Input", "PostDynamics", "PostHarmony" };

static const char* const kChorusModeStrings[] = { "Chorus", "Ensemble", "Dimension", "Microshift" };

static const char* const kDriveModeStrings[] = { "Warm", "Overdrive", "Megaphone" };

const char* LibrarySerializer::serializeEnumString(const ParamDescriptor* desc, int value) {
    if (!desc || value < 0) return nullptr;

    switch (desc->wireId) {
        case 259u: // harmony.mode (0..2)
            if (value < 3) return kHarmonyModeStrings[value];
            break;
        case 260u: // harmony.key (0..11)
            if (value < 12) return kHarmonyKeyStrings[value];
            break;
        case 261u: // harmony.scale (0..11)
            if (value < 12) return kHarmonyScaleStrings[value];
            break;
        case 274u: // harmony.voice1.non_scale_policy (0..2)
            if (value < 3) return kNonScalePolicyStrings[value];
            break;
        case 776u:  // delay.left_subdivision (0..12)
        case 777u:  // delay.right_subdivision (0..12)
        case 1541u: // chorus.subdivision (0..12)
            if (value < 13) return kTempoSubdivisionStrings[value];
            break;
        case 1282u: // output.spatial_routing (0..1)
            if (value < 2) return kSpatialRoutingStrings[value];
            break;
        case 1283u: // output.spatial_source (0..2)
            if (value < 3) return kSpatialSourceStrings[value];
            break;
        case 1537u: // chorus.mode (0..3: Chorus=0, Ensemble=1, Dimension=2, Microshift=3)
            if (value < 4) return kChorusModeStrings[value];
            break;
        case 1793u: // drive.mode (0..2: Warm=0, Overdrive=1, Megaphone=2)
            if (value < 3) return kDriveModeStrings[value];
            break;
        default:
            break;
    }
    return nullptr;
}

int LibrarySerializer::parseEnumString(const ParamDescriptor* desc, const char* str) {
    if (!desc || !str) return -1;
    char norm[32];
    toLowerString(str, norm, sizeof(norm));

    // 1. Harmony Mode: Fixed (0), Diatonic (1), Midi (2)
    if (desc->wireId == 259u) {
        if (std::strcmp(norm, "fixed") == 0 || std::strcmp(norm, "fixedinterval") == 0) return 0;
        if (std::strcmp(norm, "diatonic") == 0) return 1;
        if (std::strcmp(norm, "midi") == 0 || std::strcmp(norm, "midichord") == 0) return 2;
    }

    // 2. Harmony Key: C (0), C# (1), D (2), D# (3), E (4), F (5), F# (6), G (7), G# (8), A (9), A# (10), B (11)
    if (desc->wireId == 260u) {
        if (std::strcmp(norm, "c") == 0) return 0;
        if (std::strcmp(norm, "c#") == 0 || std::strcmp(norm, "db") == 0) return 1;
        if (std::strcmp(norm, "d") == 0) return 2;
        if (std::strcmp(norm, "d#") == 0 || std::strcmp(norm, "eb") == 0) return 3;
        if (std::strcmp(norm, "e") == 0) return 4;
        if (std::strcmp(norm, "f") == 0) return 5;
        if (std::strcmp(norm, "f#") == 0 || std::strcmp(norm, "gb") == 0) return 6;
        if (std::strcmp(norm, "g") == 0) return 7;
        if (std::strcmp(norm, "g#") == 0 || std::strcmp(norm, "ab") == 0) return 8;
        if (std::strcmp(norm, "a") == 0) return 9;
        if (std::strcmp(norm, "a#") == 0 || std::strcmp(norm, "bb") == 0) return 10;
        if (std::strcmp(norm, "b") == 0) return 11;
    }

    // 3. Harmony Scale (ScaleType in scale.h):
    // Major=0, NaturalMinor=1, HarmonicMinor=2, MelodicMinor=3, Dorian=4, Phrygian=5,
    // Lydian=6, Mixolydian=7, Locrian=8, MajorPentatonic=9, MinorPentatonic=10, BluesMinor=11
    if (desc->wireId == 261u) {
        if (std::strcmp(norm, "major") == 0) return 0;
        if (std::strcmp(norm, "naturalminor") == 0 || std::strcmp(norm, "minor") == 0 || std::strcmp(norm, "natmin") == 0) return 1;
        if (std::strcmp(norm, "harmonicminor") == 0 || std::strcmp(norm, "harmmin") == 0) return 2;
        if (std::strcmp(norm, "melodicminor") == 0 || std::strcmp(norm, "melmin") == 0) return 3;
        if (std::strcmp(norm, "dorian") == 0) return 4;
        if (std::strcmp(norm, "phrygian") == 0) return 5;
        if (std::strcmp(norm, "lydian") == 0) return 6;
        if (std::strcmp(norm, "mixolydian") == 0 || std::strcmp(norm, "mixolyd") == 0) return 7;
        if (std::strcmp(norm, "locrian") == 0) return 8;
        if (std::strcmp(norm, "majorpentatonic") == 0 || std::strcmp(norm, "pentatonicmajor") == 0 || std::strcmp(norm, "majpent") == 0) return 9;
        if (std::strcmp(norm, "minorpentatonic") == 0 || std::strcmp(norm, "pentatonicminor") == 0 || std::strcmp(norm, "minpent") == 0) return 10;
        if (std::strcmp(norm, "bluesminor") == 0 || std::strcmp(norm, "blues") == 0 || std::strcmp(norm, "bluesmin") == 0) return 11;
    }

    // 4. Harmony Non-Scale Policy (NonScaleNotePolicy in harmony_engine.h):
    // NearestScale=0, PreserveChromatic=1, BypassHarmony=2
    if (desc->wireId == 274u) {
        if (std::strcmp(norm, "nearest") == 0 || std::strcmp(norm, "nearestscale") == 0 || std::strcmp(norm, "ignore") == 0) return 0;
        if (std::strcmp(norm, "chromatic") == 0 || std::strcmp(norm, "preservechromatic") == 0 || std::strcmp(norm, "round") == 0) return 1;
        if (std::strcmp(norm, "bypass") == 0 || std::strcmp(norm, "bypassharmony") == 0 || std::strcmp(norm, "mute") == 0) return 2;
    }

    // 5. Tempo Subdivisions (TempoSubdivision in tempo.h): 0..12
    if (desc->wireId == 776u || desc->wireId == 777u || desc->wireId == 1541u) {
        if (std::strcmp(norm, "whole") == 0 || std::strcmp(norm, "1/1") == 0) return 0;
        if (std::strcmp(norm, "half") == 0 || std::strcmp(norm, "1/2") == 0) return 1;
        if (std::strcmp(norm, "quarter") == 0 || std::strcmp(norm, "1/4") == 0) return 2;
        if (std::strcmp(norm, "eighth") == 0 || std::strcmp(norm, "1/8") == 0) return 3;
        if (std::strcmp(norm, "sixteenth") == 0 || std::strcmp(norm, "1/16") == 0) return 4;
        if (std::strcmp(norm, "thirtysecond") == 0 || std::strcmp(norm, "1/32") == 0) return 5;
        if (std::strcmp(norm, "dottedhalf") == 0 || std::strcmp(norm, "1/2d") == 0) return 6;
        if (std::strcmp(norm, "dottedquarter") == 0 || std::strcmp(norm, "1/4d") == 0) return 7;
        if (std::strcmp(norm, "dottedeighth") == 0 || std::strcmp(norm, "1/8d") == 0) return 8;
        if (std::strcmp(norm, "dottedsixteenth") == 0 || std::strcmp(norm, "1/16d") == 0) return 9;
        if (std::strcmp(norm, "tripletquarter") == 0 || std::strcmp(norm, "1/4t") == 0) return 10;
        if (std::strcmp(norm, "tripleteighth") == 0 || std::strcmp(norm, "1/8t") == 0) return 11;
        if (std::strcmp(norm, "tripletsixteenth") == 0 || std::strcmp(norm, "1/16t") == 0) return 12;
    }

    // 6. Output Spatial Routing: Parallel (0), DelayIntoReverb (1)
    if (desc->wireId == 1282u) {
        if (std::strcmp(norm, "parallel") == 0) return 0;
        if (std::strcmp(norm, "delayintoreverb") == 0 || std::strcmp(norm, "serial") == 0) return 1;
    }

    // 7. Output Spatial Source: Input (0), PostDynamics (1), PostHarmony (2)
    if (desc->wireId == 1283u) {
        if (std::strcmp(norm, "input") == 0) return 0;
        if (std::strcmp(norm, "postdynamics") == 0) return 1;
        if (std::strcmp(norm, "postharmony") == 0) return 2;
    }

    // 8. Chorus / Modulation Mode (ChorusMode in chorus.h):
    // Chorus=0, Ensemble=1, Dimension=2, Microshift=3
    if (desc->wireId == 1537u) {
        if (std::strcmp(norm, "chorus") == 0) return 0;
        if (std::strcmp(norm, "ensemble") == 0) return 1;
        if (std::strcmp(norm, "dimension") == 0) return 2;
        if (std::strcmp(norm, "microshift") == 0) return 3;
    }

    // 9. Drive Mode (DriveMode in vocal_drive.h):
    // Warm=0, Overdrive=1, Megaphone=2
    if (desc->wireId == 1793u) {
        if (std::strcmp(norm, "warm") == 0) return 0;
        if (std::strcmp(norm, "overdrive") == 0 || std::strcmp(norm, "crunch") == 0) return 1;
        if (std::strcmp(norm, "megaphone") == 0 || std::strcmp(norm, "lead") == 0) return 2;
    }

    // Fallback: numeric integer string if present
    char* endptr = nullptr;
    long val = std::strtol(str, &endptr, 10);
    if (endptr != str && *endptr == '\0') {
        if (val >= static_cast<long>(desc->minValue) && val <= static_cast<long>(desc->maxValue)) {
            return static_cast<int>(val);
        }
    }

    return -1;
}

static bool parseParameterObject(JsonObjectConst paramsObj, CompactParamSet& outSet, std::string& outError) {
    for (JsonPairConst kv : paramsObj) {
        const char* key = kv.key().c_str();
        const ParamDescriptor* desc = ParameterRegistry::getByName(key);
        if (!desc) {
            outError = std::string("Unknown parameter: '") + key + "'";
            return false;
        }

        JsonVariantConst val = kv.value();
        ParameterValue pv;

        if (desc->type == ParamType::Enum) {
            if (val.is<const char*>()) {
                const char* s = val.as<const char*>();
                int enumVal = LibrarySerializer::parseEnumString(desc, s);
                if (enumVal >= 0) {
                    pv = ParameterValue::makeEnum(enumVal);
                } else {
                    outError = std::string("Invalid enum value for parameter '") + key + "': '" + s + "'";
                    return false;
                }
            } else if (val.is<int32_t>()) {
                int32_t i = val.as<int32_t>();
                if (i < static_cast<int32_t>(desc->minValue) || i > static_cast<int32_t>(desc->maxValue)) {
                    outError = std::string("Out-of-range enum ordinal for parameter '") + key + "': " + std::to_string(i);
                    return false;
                }
                pv = ParameterValue::makeEnum(i);
            } else if (val.is<float>()) {
                int32_t i = static_cast<int32_t>(std::round(val.as<float>()));
                if (i < static_cast<int32_t>(desc->minValue) || i > static_cast<int32_t>(desc->maxValue)) {
                    outError = std::string("Out-of-range enum ordinal for parameter '") + key + "': " + std::to_string(i);
                    return false;
                }
                pv = ParameterValue::makeEnum(i);
            } else {
                outError = std::string("Invalid value type for enum parameter: '") + key + "'";
                return false;
            }
        } else if (val.is<bool>()) {
            if (desc->type != ParamType::Bool) {
                outError = std::string("Type mismatch: boolean provided for non-bool parameter '") + key + "'";
                return false;
            }
            pv = ParameterValue::makeBool(val.as<bool>());
        } else if (val.is<const char*>()) {
            const char* s = val.as<const char*>();
            char* endptr = nullptr;
            float f = std::strtof(s, &endptr);
            if (endptr == s || *endptr != '\0') {
                outError = std::string("Invalid numeric string for parameter '") + key + "': '" + s + "'";
                return false;
            }
            if (desc->type == ParamType::Int) pv = ParameterValue::makeInt(static_cast<int32_t>(std::round(f)));
            else pv = ParameterValue::makeFloat(f);
        } else if (val.is<int32_t>()) {
            int32_t i = val.as<int32_t>();
            if (desc->type == ParamType::Bool) pv = ParameterValue::makeBool(i != 0);
            else if (desc->type == ParamType::Int) pv = ParameterValue::makeInt(i);
            else pv = ParameterValue::makeFloat(static_cast<float>(i));
        } else if (val.is<float>()) {
            float f = val.as<float>();
            if (desc->type == ParamType::Bool) pv = ParameterValue::makeBool(f > 0.5f);
            else if (desc->type == ParamType::Int) pv = ParameterValue::makeInt(static_cast<int32_t>(std::round(f)));
            else pv = ParameterValue::makeFloat(f);
        } else {
            outError = std::string("Unsupported JSON value type for parameter: '") + key + "'";
            return false;
        }

        outSet.set(desc->denseIndex, pv);
    }
    return true;
}

bool LibrarySerializer::deserializeJson(const char* jsonStr, size_t len, Library& outLibrary, std::string& outError) {
    if (!jsonStr || len == 0) {
        outError = "Empty JSON input";
        return false;
    }

    size_t docCapacity = std::max<size_t>(8192, len * 3);
    DynamicJsonDocument doc(docCapacity);
    DeserializationError err = ::deserializeJson(doc, jsonStr, len);
    if (err) {
        outError = std::string("JSON parse error: ") + err.c_str();
        return false;
    }

    if (!doc.is<JsonObject>()) {
        outError = "Root JSON must be an object";
        return false;
    }

    JsonObject root = doc.as<JsonObject>();

    outLibrary = Library();
    // Do NOT inject silent defaults for required envelope fields
    outLibrary.format = root.containsKey("format") ? (root["format"] | "") : "";
    outLibrary.formatVersion = root.containsKey("formatVersion") ? (root["formatVersion"] | 0) : 0;
    outLibrary.schemaVersion = root.containsKey("schemaVersion") ? (root["schemaVersion"] | 0) : 0;
    outLibrary.libraryId = root.containsKey("libraryId") ? (root["libraryId"] | "") : "";
    outLibrary.name = root.containsKey("name") ? (root["name"] | "") : "";

    // Presets
    if (root.containsKey("presets") && root["presets"].is<JsonArrayConst>()) {
        for (JsonObjectConst pObj : root["presets"].as<JsonArrayConst>()) {
            Preset p;
            p.id = pObj["id"] | "";
            p.name = pObj["name"] | "";
            if (pObj.containsKey("parameters") && pObj["parameters"].is<JsonObjectConst>()) {
                if (!parseParameterObject(pObj["parameters"].as<JsonObjectConst>(), p.overrides, outError)) {
                    return false;
                }
            }
            outLibrary.presets.push_back(p);
        }
    }

    // Scenes
    if (root.containsKey("scenes") && root["scenes"].is<JsonArrayConst>()) {
        for (JsonObjectConst sObj : root["scenes"].as<JsonArrayConst>()) {
            Scene s;
            s.id = sObj["id"] | "";
            s.name = sObj["name"] | "";
            s.basePresetId = sObj["basePresetId"] | "";

            if (sObj.containsKey("metadata") && sObj["metadata"].is<JsonObjectConst>()) {
                JsonObjectConst mObj = sObj["metadata"].as<JsonObjectConst>();
                s.metadata.artist = mObj["artist"] | "";
                s.metadata.notes = mObj["notes"] | "";
                s.metadata.tags = mObj["tags"] | "";
            }

            if (sObj.containsKey("parameters") && sObj["parameters"].is<JsonObjectConst>()) {
                if (!parseParameterObject(sObj["parameters"].as<JsonObjectConst>(), s.overrides, outError)) {
                    return false;
                }
            }

            if (sObj.containsKey("subscenes") && sObj["subscenes"].is<JsonArrayConst>()) {
                for (JsonObjectConst subObj : sObj["subscenes"].as<JsonArrayConst>()) {
                    Subscene sub;
                    sub.id = subObj["id"] | "";
                    sub.name = subObj["name"] | "";
                    if (subObj.containsKey("parameters") && subObj["parameters"].is<JsonObjectConst>()) {
                        if (!parseParameterObject(subObj["parameters"].as<JsonObjectConst>(), sub.overrides, outError)) {
                            return false;
                        }
                    }
                    s.subscenes.push_back(sub);
                }
            }

            outLibrary.scenes.push_back(s);
        }
    }

    // Setlists
    if (root.containsKey("setlists") && root["setlists"].is<JsonArrayConst>()) {
        for (JsonObjectConst slObj : root["setlists"].as<JsonArrayConst>()) {
            Setlist sl;
            sl.id = slObj["id"] | "";
            sl.name = slObj["name"] | "";
            if (slObj.containsKey("entries") && slObj["entries"].is<JsonArrayConst>()) {
                for (JsonObjectConst eObj : slObj["entries"].as<JsonArrayConst>()) {
                    SetlistEntry entry;
                    entry.id = eObj["id"] | "";
                    entry.sceneId = eObj["sceneId"] | "";
                    sl.entries.push_back(entry);
                }
            }
            outLibrary.setlists.push_back(sl);
        }
    }

    return true;
}

static bool serializeParameterObject(const CompactParamSet& set, JsonObject outObj, std::string& outError) {
    for (size_t i = 0; i < CompactParamSet::kMaxParams; ++i) {
        if (set.has(i)) {
            const ParamDescriptor* desc = ParameterRegistry::getByIndex(i);
            if (!desc) continue;
            ParameterValue pv = set.get(i);
            const char* key = desc->semanticName;

            switch (desc->type) {
                case ParamType::Bool:
                    outObj[key] = pv.asBool();
                    break;
                case ParamType::Int:
                    outObj[key] = pv.asInt();
                    break;
                case ParamType::Enum: {
                    const char* enumStr = LibrarySerializer::serializeEnumString(desc, pv.asInt());
                    if (!enumStr) {
                        outError = "Invalid enum ordinal for parameter '" + std::string(key) + "': " + std::to_string(pv.asInt());
                        return false;
                    }
                    outObj[key] = enumStr;
                    break;
                }
                case ParamType::Float:
                    outObj[key] = pv.asFloat();
                    break;
            }
        }
    }
    return true;
}

bool LibrarySerializer::serializeJson(const Library& library, std::string& outJson, bool pretty, std::string* outError) {
    DynamicJsonDocument doc(65536);

    doc["format"] = library.format;
    doc["formatVersion"] = library.formatVersion;
    doc["schemaVersion"] = library.schemaVersion;
    doc["libraryId"] = library.libraryId;
    doc["name"] = library.name;

    std::string err;

    JsonArray presetsArr = doc.createNestedArray("presets");
    for (const auto& p : library.presets) {
        JsonObject pObj = presetsArr.createNestedObject();
        pObj["id"] = p.id;
        pObj["name"] = p.name;
        JsonObject paramsObj = pObj.createNestedObject("parameters");
        if (!serializeParameterObject(p.overrides, paramsObj, err)) {
            if (outError) *outError = err;
            return false;
        }
    }

    JsonArray scenesArr = doc.createNestedArray("scenes");
    for (const auto& s : library.scenes) {
        JsonObject sObj = scenesArr.createNestedObject();
        sObj["id"] = s.id;
        sObj["name"] = s.name;
        if (!s.basePresetId.empty()) {
            sObj["basePresetId"] = s.basePresetId;
        }

        if (!s.metadata.artist.empty() || !s.metadata.notes.empty() || !s.metadata.tags.empty()) {
            JsonObject mObj = sObj.createNestedObject("metadata");
            if (!s.metadata.artist.empty()) mObj["artist"] = s.metadata.artist;
            if (!s.metadata.notes.empty()) mObj["notes"] = s.metadata.notes;
            if (!s.metadata.tags.empty()) mObj["tags"] = s.metadata.tags;
        }

        JsonObject sceneParamsObj = sObj.createNestedObject("parameters");
        if (!serializeParameterObject(s.overrides, sceneParamsObj, err)) {
            if (outError) *outError = err;
            return false;
        }

        JsonArray subscenesArr = sObj.createNestedArray("subscenes");
        for (const auto& sub : s.subscenes) {
            JsonObject subObj = subscenesArr.createNestedObject();
            subObj["id"] = sub.id;
            subObj["name"] = sub.name;
            JsonObject subParamsObj = subObj.createNestedObject("parameters");
            if (!serializeParameterObject(sub.overrides, subParamsObj, err)) {
                if (outError) *outError = err;
                return false;
            }
        }
    }

    JsonArray setlistsArr = doc.createNestedArray("setlists");
    for (const auto& sl : library.setlists) {
        JsonObject slObj = setlistsArr.createNestedObject();
        slObj["id"] = sl.id;
        slObj["name"] = sl.name;
        JsonArray entriesArr = slObj.createNestedArray("entries");
        for (const auto& e : sl.entries) {
            JsonObject eObj = entriesArr.createNestedObject();
            eObj["id"] = e.id;
            eObj["sceneId"] = e.sceneId;
        }
    }

    outJson.clear();
    if (pretty) {
        serializeJsonPretty(doc, outJson);
    } else {
        ::serializeJson(doc, outJson);
    }
    return true;
}
