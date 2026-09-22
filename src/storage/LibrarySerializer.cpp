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

static int parseEnumString(const ParamDescriptor* desc, const char* str) {
    if (!desc || !str) return -1;
    char norm[32];
    toLowerString(str, norm, sizeof(norm));

    // Harmony Key: C, C#, Db, D, D#, Eb, E, F, F#, Gb, G, G#, Ab, A, A#, Bb, B
    if (desc->wireId == 260u) { // harmony.key
        static const char* const kKeyNames[] = {
            "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"
        };
        // Also check flat equivalents
        if (std::strcmp(norm, "db") == 0) return 1;
        if (std::strcmp(norm, "eb") == 0) return 3;
        if (std::strcmp(norm, "gb") == 0) return 6;
        if (std::strcmp(norm, "ab") == 0) return 8;
        if (std::strcmp(norm, "bb") == 0) return 10;
        for (int i = 0; i < 12; ++i) {
            char knorm[16];
            toLowerString(kKeyNames[i], knorm, sizeof(knorm));
            if (std::strcmp(norm, knorm) == 0) return i;
        }
    }

    // Harmony Scale
    if (desc->wireId == 261u) { // harmony.scale
        static const char* const kScaleNames[] = {
            "major", "minor", "dorian", "phrygian", "lydian", "mixolydian",
            "locrian", "harmonicminor", "melodicminor", "pentatonicmajor",
            "pentatonicminor", "blues"
        };
        for (int i = 0; i < 12; ++i) {
            if (std::strcmp(norm, kScaleNames[i]) == 0) return i;
        }
    }

    // Harmony Mode: Fixed (0), Diatonic (1), Midi (2)
    if (desc->wireId == 259u) { // harmony.mode
        if (std::strcmp(norm, "fixed") == 0) return 0;
        if (std::strcmp(norm, "diatonic") == 0) return 1;
        if (std::strcmp(norm, "midi") == 0) return 2;
    }

    // Chorus / Modulation Mode: Dimension (0), Chorus (1), Ensemble (2), Microshift (3)
    if (desc->wireId == 1537u) { // chorus.mode
        if (std::strcmp(norm, "dimension") == 0) return 0;
        if (std::strcmp(norm, "chorus") == 0) return 1;
        if (std::strcmp(norm, "ensemble") == 0) return 2;
        if (std::strcmp(norm, "microshift") == 0) return 3;
    }

    // Non Scale Policy: Ignore (0), Round (1), Mute (2)
    if (desc->wireId == 274u) {
        if (std::strcmp(norm, "ignore") == 0) return 0;
        if (std::strcmp(norm, "round") == 0) return 1;
        if (std::strcmp(norm, "mute") == 0) return 2;
    }

    // Drive Mode: Warm (0), Crunch (1), Lead (2)
    if (desc->wireId == 1793u) {
        if (std::strcmp(norm, "warm") == 0) return 0;
        if (std::strcmp(norm, "crunch") == 0) return 1;
        if (std::strcmp(norm, "lead") == 0) return 2;
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

        if (val.is<bool>()) {
            pv = ParameterValue::makeBool(val.as<bool>());
        } else if (val.is<const char*>()) {
            const char* s = val.as<const char*>();
            int enumVal = parseEnumString(desc, s);
            if (enumVal >= 0) {
                pv = ParameterValue::makeEnum(enumVal);
            } else {
                char* endptr = nullptr;
                float f = std::strtof(s, &endptr);
                if (endptr == s) {
                    outError = std::string("Invalid value for parameter '") + key + "': '" + s + "'";
                    return false;
                }
                pv = ParameterValue::makeFloat(f);
            }
        } else if (val.is<int32_t>()) {
            int32_t i = val.as<int32_t>();
            if (desc->type == ParamType::Bool) pv = ParameterValue::makeBool(i != 0);
            else if (desc->type == ParamType::Enum) pv = ParameterValue::makeEnum(i);
            else if (desc->type == ParamType::Int) pv = ParameterValue::makeInt(i);
            else pv = ParameterValue::makeFloat(static_cast<float>(i));
        } else if (val.is<float>()) {
            float f = val.as<float>();
            if (desc->type == ParamType::Bool) pv = ParameterValue::makeBool(f > 0.5f);
            else if (desc->type == ParamType::Enum) pv = ParameterValue::makeEnum(static_cast<int32_t>(f));
            else if (desc->type == ParamType::Int) pv = ParameterValue::makeInt(static_cast<int32_t>(f));
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
    outLibrary.format = root["format"] | "voxp4-library";
    outLibrary.formatVersion = root["formatVersion"] | 1;
    outLibrary.schemaVersion = root["schemaVersion"] | 1;
    outLibrary.libraryId = root["libraryId"] | "";
    outLibrary.name = root["name"] | "";

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

static void serializeParameterObject(const CompactParamSet& set, JsonObject outObj) {
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
                case ParamType::Enum:
                    outObj[key] = pv.asInt();
                    break;
                case ParamType::Float:
                    outObj[key] = pv.asFloat();
                    break;
            }
        }
    }
}

bool LibrarySerializer::serializeJson(const Library& library, std::string& outJson, bool pretty) {
    DynamicJsonDocument doc(65536);

    doc["format"] = library.format;
    doc["formatVersion"] = library.formatVersion;
    doc["schemaVersion"] = library.schemaVersion;
    doc["libraryId"] = library.libraryId;
    doc["name"] = library.name;

    JsonArray presetsArr = doc.createNestedArray("presets");
    for (const auto& p : library.presets) {
        JsonObject pObj = presetsArr.createNestedObject();
        pObj["id"] = p.id;
        pObj["name"] = p.name;
        JsonObject paramsObj = pObj.createNestedObject("parameters");
        serializeParameterObject(p.overrides, paramsObj);
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
        serializeParameterObject(s.overrides, sceneParamsObj);

        JsonArray subscenesArr = sObj.createNestedArray("subscenes");
        for (const auto& sub : s.subscenes) {
            JsonObject subObj = subscenesArr.createNestedObject();
            subObj["id"] = sub.id;
            subObj["name"] = sub.name;
            JsonObject subParamsObj = subObj.createNestedObject("parameters");
            serializeParameterObject(sub.overrides, subParamsObj);
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
