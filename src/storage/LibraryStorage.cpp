#include "LibraryStorage.h"
#include "LibrarySerializer.h"
#include "LibraryValidator.h"
#include "model/ParameterRegistry.h"
#include <cstdio>
#include <fstream>
#include <sstream>

#if defined(ARDUINO) && !defined(PIOTEST)
#include <LittleFS.h>
#define USE_LITTLEFS 1
static const char* kDefaultPath = "/library.voxp4.json";
static const char* kDefaultTmpPath = "/library.voxp4.tmp.json";
#else
#define USE_LITTLEFS 0
static const char* kDefaultPath = "library.voxp4.json";
static const char* kDefaultTmpPath = "library.voxp4.tmp.json";
#endif

bool LibraryStorage::init() {
#if USE_LITTLEFS
    if (!LittleFS.begin(true)) {
        return false;
    }
#endif
    return true;
}

Library LibraryStorage::createFactoryLibrary() {
    Library lib;
    lib.format = "voxp4-library";
    lib.formatVersion = 1;
    lib.schemaVersion = 1;
    lib.libraryId = "factory-default";
    lib.name = "VoxP4 Factory Rig";

    // Preset 1: Clean Vocal
    Preset p1;
    p1.id = "preset-clean-vocal";
    p1.name = "Clean Vocal";
    // Harmony off, Reverb low, Delay off
    const ParamDescriptor* dHarmEn = ParameterRegistry::getByName("HarmonyEnable");
    if (dHarmEn) p1.overrides.set(dHarmEn->denseIndex, ParameterValue::makeBool(false));
    const ParamDescriptor* dRevWet = ParameterRegistry::getByName("ReverbWet");
    if (dRevWet) p1.overrides.set(dRevWet->denseIndex, ParameterValue::makeFloat(0.12f));
    const ParamDescriptor* dDelayEn = ParameterRegistry::getByName("DelayEnable");
    if (dDelayEn) p1.overrides.set(dDelayEn->denseIndex, ParameterValue::makeBool(false));
    lib.presets.push_back(p1);

    // Preset 2: Wide Harmony
    Preset p2;
    p2.id = "preset-wide-harmony";
    p2.name = "Wide Harmony";
    if (dHarmEn) p2.overrides.set(dHarmEn->denseIndex, ParameterValue::makeBool(true));
    const ParamDescriptor* dHarmLev = ParameterRegistry::getByName("HarmonyLevel");
    if (dHarmLev) p2.overrides.set(dHarmLev->denseIndex, ParameterValue::makeFloat(0.85f));
    const ParamDescriptor* dModMode = ParameterRegistry::getByName("ModulationMode");
    if (dModMode) p2.overrides.set(dModMode->denseIndex, ParameterValue::makeEnum(3)); // Microshift
    const ParamDescriptor* dModMix = ParameterRegistry::getByName("ModulationMix");
    if (dModMix) p2.overrides.set(dModMix->denseIndex, ParameterValue::makeFloat(0.35f));
    lib.presets.push_back(p2);

    // Scene 1: Song A
    Scene s1;
    s1.id = "scene-song-a";
    s1.name = "Song A";
    s1.basePresetId = "preset-clean-vocal";
    s1.metadata.artist = "Demo Artist";
    s1.metadata.notes = "Mid-tempo rock";
    const ParamDescriptor* dBpm = ParameterRegistry::getByName("TempoBpm");
    if (dBpm) s1.overrides.set(dBpm->denseIndex, ParameterValue::makeFloat(120.0f));
    const ParamDescriptor* dKey = ParameterRegistry::getByName("HarmonyKey");
    if (dKey) s1.overrides.set(dKey->denseIndex, ParameterValue::makeEnum(0)); // C

    // Subscene 1: Intro
    Subscene sub1;
    sub1.id = "subscene-song-a-intro";
    sub1.name = "Intro";
    if (dHarmEn) sub1.overrides.set(dHarmEn->denseIndex, ParameterValue::makeBool(false));
    s1.subscenes.push_back(sub1);

    // Subscene 2: Verse
    Subscene sub2;
    sub2.id = "subscene-song-a-verse";
    sub2.name = "Verse";
    if (dHarmEn) sub2.overrides.set(dHarmEn->denseIndex, ParameterValue::makeBool(false));
    if (dRevWet) sub2.overrides.set(dRevWet->denseIndex, ParameterValue::makeFloat(0.15f));
    s1.subscenes.push_back(sub2);

    // Subscene 3: Chorus
    Subscene sub3;
    sub3.id = "subscene-song-a-chorus";
    sub3.name = "Chorus";
    if (dHarmEn) sub3.overrides.set(dHarmEn->denseIndex, ParameterValue::makeBool(true));
    if (dRevWet) sub3.overrides.set(dRevWet->denseIndex, ParameterValue::makeFloat(0.28f));
    const ParamDescriptor* dDelWet = ParameterRegistry::getByName("DelayWet");
    if (dDelWet) sub3.overrides.set(dDelWet->denseIndex, ParameterValue::makeFloat(0.22f));
    if (dDelayEn) sub3.overrides.set(dDelayEn->denseIndex, ParameterValue::makeBool(true));
    s1.subscenes.push_back(sub3);

    lib.scenes.push_back(s1);

    // Scene 2: Song B
    Scene s2;
    s2.id = "scene-song-b";
    s2.name = "Song B";
    s2.basePresetId = "preset-wide-harmony";
    s2.metadata.artist = "Demo Artist";
    if (dBpm) s2.overrides.set(dBpm->denseIndex, ParameterValue::makeFloat(95.0f));
    if (dKey) s2.overrides.set(dKey->denseIndex, ParameterValue::makeEnum(4)); // E

    Subscene subB1;
    subB1.id = "subscene-song-b-verse";
    subB1.name = "Verse";
    if (dHarmEn) subB1.overrides.set(dHarmEn->denseIndex, ParameterValue::makeBool(false));
    s2.subscenes.push_back(subB1);

    Subscene subB2;
    subB2.id = "subscene-song-b-chorus";
    subB2.name = "Chorus";
    if (dHarmEn) subB2.overrides.set(dHarmEn->denseIndex, ParameterValue::makeBool(true));
    s2.subscenes.push_back(subB2);

    lib.scenes.push_back(s2);

    // Setlist: Main Setlist
    Setlist sl;
    sl.id = "setlist-tour";
    sl.name = "Tour Setlist";
    sl.entries.push_back({ "entry-1", "scene-song-a" });
    sl.entries.push_back({ "entry-2", "scene-song-b" });
    lib.setlists.push_back(sl);

    return lib;
}

bool LibraryStorage::loadLibrary(Library& outLibrary, const char* path) {
    const char* filePath = path ? path : kDefaultPath;
    std::string jsonContent;

#if USE_LITTLEFS
    File f = LittleFS.open(filePath, "r");
    if (!f) {
        outLibrary = createFactoryLibrary();
        saveLibraryAtomic(outLibrary, filePath);
        return true;
    }
    while (f.available()) {
        jsonContent += static_cast<char>(f.read());
    }
    f.close();
#else
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        outLibrary = createFactoryLibrary();
        return true;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    jsonContent = ss.str();
#endif

    std::string error;
    Library loaded;
    if (!LibrarySerializer::deserializeJson(jsonContent, loaded, error)) {
        // Corrupt file fallback
        outLibrary = createFactoryLibrary();
        return false;
    }

    ValidationResult val = LibraryValidator::validate(loaded);
    if (!val.isValid()) {
        outLibrary = createFactoryLibrary();
        return false;
    }

    outLibrary = loaded;
    return true;
}

bool LibraryStorage::saveLibraryAtomic(const Library& library, const char* path) {
    ValidationResult val = LibraryValidator::validate(library);
    if (!val.isValid()) {
        return false;
    }

    std::string jsonStr;
    if (!LibrarySerializer::serializeJson(library, jsonStr, true)) {
        return false;
    }

    const char* targetPath = path ? path : kDefaultPath;
    const char* tmpPath = kDefaultTmpPath;

#if USE_LITTLEFS
    File tmpFile = LittleFS.open(tmpPath, "w");
    if (!tmpFile) return false;
    size_t written = tmpFile.write(reinterpret_cast<const uint8_t*>(jsonStr.data()), jsonStr.size());
    tmpFile.flush();
    tmpFile.close();

    if (written != jsonStr.size()) {
        LittleFS.remove(tmpPath);
        return false;
    }

    // Atomic rename
    LittleFS.remove(targetPath);
    return LittleFS.rename(tmpPath, targetPath);
#else
    {
        std::ofstream ofs(tmpPath, std::ios::trunc);
        if (!ofs.is_open()) return false;
        ofs << jsonStr;
        ofs.flush();
        if (!ofs.good()) {
            std::remove(tmpPath);
            return false;
        }
    }

    std::remove(targetPath);
    return std::rename(tmpPath, targetPath) == 0;
#endif
}
