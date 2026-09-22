#include <unity.h>
#include "model/ParameterValue.h"
#include "model/CompactParamSet.h"
#include "model/ParameterRegistry.h"
#include "model/LibraryModel.h"
#include "session/ResolvedState.h"
#include "session/StateResolver.h"
#include "session/ParameterDiff.h"
#include "session/PerformanceSession.h"
#include "storage/LibraryValidator.h"
#include "storage/LibrarySerializer.h"
#include "storage/LibraryStorage.h"
#include <ArduinoJson.h>
#include <fstream>
#include <set>
#include <sstream>

void setUp(void) {}
void tearDown(void) {}

void test_parameter_value_types(void) {
    ParameterValue b = ParameterValue::makeBool(true);
    TEST_ASSERT_TRUE(b.asBool());
    TEST_ASSERT_EQUAL_INT(1, b.asInt());
    TEST_ASSERT_EQUAL_FLOAT(1.0f, b.asFloat());

    ParameterValue i = ParameterValue::makeInt(-7);
    TEST_ASSERT_EQUAL_INT(-7, i.asInt());
    TEST_ASSERT_EQUAL_FLOAT(-7.0f, i.asFloat());

    ParameterValue f = ParameterValue::makeFloat(0.25f);
    TEST_ASSERT_EQUAL_FLOAT(0.25f, f.asFloat());

    ParameterValue b2 = ParameterValue::makeBool(true);
    TEST_ASSERT_TRUE(b == b2);
    ParameterValue b3 = ParameterValue::makeBool(false);
    TEST_ASSERT_TRUE(b != b3);
}

void test_compact_param_set_operations(void) {
    CompactParamSet set;
    TEST_ASSERT_TRUE(set.isEmpty());
    TEST_ASSERT_EQUAL_UINT(0, set.count());

    set.set(0, ParameterValue::makeFloat(120.0f));
    set.set(1, ParameterValue::makeBool(true));
    set.set(70, ParameterValue::makeFloat(0.95f));

    TEST_ASSERT_FALSE(set.isEmpty());
    TEST_ASSERT_EQUAL_UINT(3, set.count());
    TEST_ASSERT_TRUE(set.has(0));
    TEST_ASSERT_TRUE(set.has(1));
    TEST_ASSERT_FALSE(set.has(2));
    TEST_ASSERT_TRUE(set.has(70));

    TEST_ASSERT_EQUAL_FLOAT(120.0f, set.get(0).asFloat());
    TEST_ASSERT_TRUE(set.get(1).asBool());
    TEST_ASSERT_EQUAL_FLOAT(0.95f, set.get(70).asFloat());

    CompactParamSet overlay;
    overlay.set(1, ParameterValue::makeBool(false));
    overlay.set(2, ParameterValue::makeInt(4));

    set.mergeFrom(overlay);
    TEST_ASSERT_EQUAL_UINT(4, set.count());
    TEST_ASSERT_FALSE(set.get(1).asBool()); // overwritten
    TEST_ASSERT_EQUAL_INT(4, set.get(2).asInt()); // added
    TEST_ASSERT_EQUAL_FLOAT(120.0f, set.get(0).asFloat()); // preserved
}

void test_parameter_registry_lookups(void) {
    // Lookup by dense index
    const ParamDescriptor* d0 = ParameterRegistry::getByIndex(0);
    TEST_ASSERT_NOT_NULL(d0);
    TEST_ASSERT_EQUAL_STRING("tempo.bpm", d0->key);
    TEST_ASSERT_EQUAL_UINT(16, d0->wireId);

    // Lookup by wire ID
    const ParamDescriptor* dHarm = ParameterRegistry::getByWireId(256);
    TEST_ASSERT_NOT_NULL(dHarm);
    TEST_ASSERT_EQUAL_STRING("HarmonyEnable", dHarm->semanticName);

    // Case-tolerant, punctuation-tolerant name lookup
    TEST_ASSERT_EQUAL_PTR(dHarm, ParameterRegistry::getByName("harmony.enable"));
    TEST_ASSERT_EQUAL_PTR(dHarm, ParameterRegistry::getByName("HarmonyEnable"));
    TEST_ASSERT_EQUAL_PTR(dHarm, ParameterRegistry::getByName("harmony_enable"));
    TEST_ASSERT_EQUAL_PTR(dHarm, ParameterRegistry::getByName("PitchShiftEnabled"));

    const ParamDescriptor* dMicro = ParameterRegistry::getByName("MicroshiftLeftCents");
    TEST_ASSERT_NOT_NULL(dMicro);
    TEST_ASSERT_EQUAL_UINT(1545, dMicro->wireId);

    // Validation and clamping
    ParameterValue clamped = ParameterValue::makeFloat(999.0f);
    TEST_ASSERT_TRUE(ParameterRegistry::validateAndClamp(*dMicro, clamped));
    TEST_ASSERT_EQUAL_FLOAT(50.0f, clamped.asFloat()); // max is 50.0
}

void test_state_resolver_5_tier_inheritance(void) {
    // Hierarchy test:
    // DelayWet default: 0.20
    // Preset: 0.22
    // Scene: 0.25
    // Subscene: 0.30
    // TemporaryEdits: 0.35
    const ParamDescriptor* dDelayWet = ParameterRegistry::getByName("DelayWet");
    TEST_ASSERT_NOT_NULL(dDelayWet);
    size_t idx = dDelayWet->denseIndex;

    CompactParamSet preset;
    preset.set(idx, ParameterValue::makeFloat(0.22f));

    CompactParamSet scene;
    scene.set(idx, ParameterValue::makeFloat(0.25f));

    CompactParamSet subscene;
    subscene.set(idx, ParameterValue::makeFloat(0.30f));

    CompactParamSet tempEdits;
    tempEdits.set(idx, ParameterValue::makeFloat(0.35f));

    // Level 1: Defaults only
    ResolvedState s1 = StateResolver::resolveRaw(nullptr, nullptr, nullptr, nullptr);
    TEST_ASSERT_EQUAL_FLOAT(dDelayWet->defaultValue, s1.get(idx).asFloat());

    // Level 2: + Preset
    ResolvedState s2 = StateResolver::resolveRaw(&preset, nullptr, nullptr, nullptr);
    TEST_ASSERT_EQUAL_FLOAT(0.22f, s2.get(idx).asFloat());

    // Level 3: + Scene
    ResolvedState s3 = StateResolver::resolveRaw(&preset, &scene, nullptr, nullptr);
    TEST_ASSERT_EQUAL_FLOAT(0.25f, s3.get(idx).asFloat());

    // Level 4: + Subscene
    ResolvedState s4 = StateResolver::resolveRaw(&preset, &scene, &subscene, nullptr);
    TEST_ASSERT_EQUAL_FLOAT(0.30f, s4.get(idx).asFloat());

    // Level 5: + TemporaryEdits
    ResolvedState s5 = StateResolver::resolveRaw(&preset, &scene, &subscene, &tempEdits);
    TEST_ASSERT_EQUAL_FLOAT(0.35f, s5.get(idx).asFloat());
}

void test_sparse_override_isolation(void) {
    const ParamDescriptor* dHarmEn = ParameterRegistry::getByName("HarmonyEnable");
    const ParamDescriptor* dDelayWet = ParameterRegistry::getByName("DelayWet");
    const ParamDescriptor* dBpm = ParameterRegistry::getByName("TempoBpm");

    CompactParamSet subscene;
    subscene.set(dHarmEn->denseIndex, ParameterValue::makeBool(true));
    subscene.set(dDelayWet->denseIndex, ParameterValue::makeFloat(0.40f));

    ResolvedState res = StateResolver::resolveRaw(nullptr, nullptr, &subscene, nullptr);

    // Overridden params match subscene
    TEST_ASSERT_TRUE(res.get(dHarmEn->denseIndex).asBool());
    TEST_ASSERT_EQUAL_FLOAT(0.40f, res.get(dDelayWet->denseIndex).asFloat());

    // Non-overridden params remain at defaults
    TEST_ASSERT_EQUAL_FLOAT(dBpm->defaultValue, res.get(dBpm->denseIndex).asFloat());
}

void test_parameter_diff_exact_deltas(void) {
    ResolvedState sOld;
    ResolvedState sNew = sOld;

    const ParamDescriptor* dHarmEn = ParameterRegistry::getByName("HarmonyEnable");
    const ParamDescriptor* dRevWet = ParameterRegistry::getByName("ReverbWet");

    sNew.set(dHarmEn->denseIndex, ParameterValue::makeBool(true));
    sNew.set(dRevWet->denseIndex, ParameterValue::makeFloat(0.33f));

    std::vector<ParamDelta> diff = ParameterDiff::compute(sOld, sNew);
    TEST_ASSERT_EQUAL_UINT(2, diff.size());

    bool sawHarm = false, sawRev = false;
    for (const auto& d : diff) {
        if (d.wireId == dHarmEn->wireId) {
            sawHarm = true;
            TEST_ASSERT_TRUE(d.value.asBool());
        } else if (d.wireId == dRevWet->wireId) {
            sawRev = true;
            TEST_ASSERT_EQUAL_FLOAT(0.33f, d.value.asFloat());
        }
    }
    TEST_ASSERT_TRUE(sawHarm);
    TEST_ASSERT_TRUE(sawRev);
}

void test_performance_session_navigation(void) {
    Library lib = LibraryStorage::createFactoryLibrary();
    PerformanceSession session(&lib);

    // Initial state points to first setlist, first scene, first subscene
    TEST_ASSERT_EQUAL_STRING("setlist-tour", session.getActiveSetlistId().c_str());
    TEST_ASSERT_EQUAL_INT(0, session.getActiveEntryIndex());
    TEST_ASSERT_EQUAL_STRING("scene-song-a", session.getActiveSceneId().c_str());
    TEST_ASSERT_EQUAL_STRING("subscene-song-a-intro", session.getActiveSubsceneId().c_str());

    // Next subscene: Intro -> Verse
    std::vector<ParamDelta> deltas = session.nextSubscene();
    TEST_ASSERT_EQUAL_STRING("subscene-song-a-verse", session.getActiveSubsceneId().c_str());

    // Next subscene: Verse -> Chorus
    deltas = session.nextSubscene();
    TEST_ASSERT_EQUAL_STRING("subscene-song-a-chorus", session.getActiveSubsceneId().c_str());
    TEST_ASSERT_TRUE(deltas.size() > 0);

    // Chorus is last subscene in Song A -> nextSubscene() must NOT wrap
    deltas = session.nextSubscene();
    TEST_ASSERT_EQUAL_STRING("subscene-song-a-chorus", session.getActiveSubsceneId().c_str());
    TEST_ASSERT_EQUAL_UINT(0, deltas.size());

    // Previous subscene: Chorus -> Verse
    session.previousSubscene();
    TEST_ASSERT_EQUAL_STRING("subscene-song-a-verse", session.getActiveSubsceneId().c_str());

    // Next scene: Song A -> Song B
    deltas = session.nextScene();
    TEST_ASSERT_EQUAL_INT(1, session.getActiveEntryIndex());
    TEST_ASSERT_EQUAL_STRING("scene-song-b", session.getActiveSceneId().c_str());
    TEST_ASSERT_EQUAL_STRING("subscene-song-b-verse", session.getActiveSubsceneId().c_str());

    // Song B is last in setlist -> nextScene() must NOT wrap
    deltas = session.nextScene();
    TEST_ASSERT_EQUAL_INT(1, session.getActiveEntryIndex());
    TEST_ASSERT_EQUAL_UINT(0, deltas.size());
}

void test_temporary_edits_dirty_and_revert(void) {
    Library lib = LibraryStorage::createFactoryLibrary();
    PerformanceSession session(&lib);

    TEST_ASSERT_FALSE(session.isDirty());

    const ParamDescriptor* dRevWet = ParameterRegistry::getByName("ReverbWet");
    float originalVal = session.getResolvedState().get(dRevWet->denseIndex).asFloat();

    // Quick edit: raise ReverbWet to 0.50
    session.applyTemporaryEdit(dRevWet->wireId, ParameterValue::makeFloat(0.50f));
    TEST_ASSERT_TRUE(session.isDirty());
    TEST_ASSERT_EQUAL_FLOAT(0.50f, session.getResolvedState().get(dRevWet->denseIndex).asFloat());

    // Revert
    std::vector<ParamDelta> revertDeltas = session.revertTemporaryEdits();
    TEST_ASSERT_FALSE(session.isDirty());
    TEST_ASSERT_EQUAL_FLOAT(originalVal, session.getResolvedState().get(dRevWet->denseIndex).asFloat());
    TEST_ASSERT_EQUAL_UINT(1, revertDeltas.size());
    TEST_ASSERT_EQUAL_UINT(dRevWet->wireId, revertDeltas[0].wireId);
    TEST_ASSERT_EQUAL_FLOAT(originalVal, revertDeltas[0].value.asFloat());
}

void test_commit_temporary_edits(void) {
    Library lib = LibraryStorage::createFactoryLibrary();
    PerformanceSession session(&lib);

    const ParamDescriptor* dRevWet = ParameterRegistry::getByName("ReverbWet");
    session.selectSubscene("subscene-song-a-chorus");

    // Apply quick edit
    session.applyTemporaryEdit(dRevWet->wireId, ParameterValue::makeFloat(0.48f));
    TEST_ASSERT_TRUE(session.isDirty());

    // Commit to active subscene
    bool ok = session.commitTemporaryEdits();
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_FALSE(session.isDirty());

    // Verify subscene in library now permanently owns 0.48
    const Subscene* sub = session.getActiveSubscene();
    TEST_ASSERT_NOT_NULL(sub);
    TEST_ASSERT_TRUE(sub->overrides.has(dRevWet->denseIndex));
    TEST_ASSERT_EQUAL_FLOAT(0.48f, sub->overrides.get(dRevWet->denseIndex).asFloat());

    // Verify base preset was NOT touched
    const Preset* p = session.getActiveBasePreset();
    TEST_ASSERT_NOT_NULL(p);
    if (p->overrides.has(dRevWet->denseIndex)) {
        TEST_ASSERT_NOT_EQUAL(0.48f, p->overrides.get(dRevWet->denseIndex).asFloat());
    }
}

void test_library_validator(void) {
    Library lib = LibraryStorage::createFactoryLibrary();
    ValidationResult v = LibraryValidator::validate(lib);
    TEST_ASSERT_TRUE(v.isValid());

    // Duplicate preset ID
    Library badLib = lib;
    badLib.presets.push_back(badLib.presets[0]);
    ValidationResult vBad = LibraryValidator::validate(badLib);
    TEST_ASSERT_FALSE(vBad.isValid());

    // Broken reference: scene points to non-existent preset
    badLib = lib;
    badLib.scenes[0].basePresetId = "non-existent-preset";
    vBad = LibraryValidator::validate(badLib);
    TEST_ASSERT_FALSE(vBad.isValid());

    // Unsupported version
    badLib = lib;
    badLib.formatVersion = 99;
    vBad = LibraryValidator::validate(badLib);
    TEST_ASSERT_FALSE(vBad.isValid());
}

void test_web_editor_golden_fixture_and_roundtrip(void) {
    std::ifstream ifs("tests/fixtures/library_v1.json");
    TEST_ASSERT_TRUE_MESSAGE(ifs.is_open(), "Could not open tests/fixtures/library_v1.json");

    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string fixtureJson = ss.str();

    Library lib;
    std::string err;
    bool ok = LibrarySerializer::deserializeJson(fixtureJson, lib, err);
    TEST_ASSERT_TRUE_MESSAGE(ok, err.c_str());

    // Validate fixture
    ValidationResult val = LibraryValidator::validate(lib);
    TEST_ASSERT_TRUE_MESSAGE(val.isValid(), val.errors.empty() ? "" : val.errors[0].c_str());

    TEST_ASSERT_EQUAL_STRING("web-demo-rig", lib.libraryId.c_str());
    TEST_ASSERT_EQUAL_UINT(2, lib.presets.size());
    TEST_ASSERT_EQUAL_UINT(2, lib.scenes.size());
    TEST_ASSERT_EQUAL_UINT(1, lib.setlists.size());

    // Test Scene 0 (Creep) has 4 subscenes
    const Scene* creep = lib.findScene("scene-creep");
    TEST_ASSERT_NOT_NULL(creep);
    TEST_ASSERT_EQUAL_UINT(4, creep->subscenes.size());

    // Verify Enum string parsing: "HarmonyKey": "G", "HarmonyScale": "Major"
    const ParamDescriptor* dKey = ParameterRegistry::getByName("HarmonyKey");
    TEST_ASSERT_TRUE(creep->overrides.has(dKey->denseIndex));
    TEST_ASSERT_EQUAL_INT(7, creep->overrides.get(dKey->denseIndex).asInt()); // G is 7

    // Roundtrip: serialize back to JSON
    std::string outJson;
    ok = LibrarySerializer::serializeJson(lib, outJson, true);
    TEST_ASSERT_TRUE(ok);

    // Re-parse output JSON
    Library roundtripLib;
    ok = LibrarySerializer::deserializeJson(outJson, roundtripLib, err);
    TEST_ASSERT_TRUE_MESSAGE(ok, err.c_str());
    TEST_ASSERT_EQUAL_STRING(lib.libraryId.c_str(), roundtripLib.libraryId.c_str());
    TEST_ASSERT_EQUAL_UINT(lib.scenes.size(), roundtripLib.scenes.size());
}

static bool loadAndValidateFixture(const char* path, std::string& outErr) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        outErr = std::string("Could not open file: ") + path;
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    Library lib;
    if (!LibrarySerializer::deserializeJson(ss.str(), lib, outErr)) {
        return false;
    }
    ValidationResult val = LibraryValidator::validate(lib);
    if (!val.isValid()) {
        outErr = val.errors.empty() ? "Validation failed" : val.errors[0];
        return false;
    }
    return true;
}

void test_71_parameter_contract_audit(void) {
    std::set<uint16_t> wireIds;
    std::set<size_t> denseIndices;
    std::set<std::string> semanticNames;

    TEST_ASSERT_EQUAL_UINT(71, VOXP4_PARAM_COUNT);

    for (size_t i = 0; i < VOXP4_PARAM_COUNT; ++i) {
        const ParamDescriptor* desc = ParameterRegistry::getByIndex(i);
        TEST_ASSERT_NOT_NULL(desc);
        TEST_ASSERT_EQUAL_UINT(i, desc->denseIndex);
        TEST_ASSERT_TRUE(desc->wireId > 0);
        TEST_ASSERT_NOT_NULL(desc->semanticName);
        TEST_ASSERT_TRUE(std::strlen(desc->semanticName) > 0);

        // Verify uniqueness
        TEST_ASSERT_TRUE_MESSAGE(wireIds.insert(desc->wireId).second, desc->semanticName);
        TEST_ASSERT_TRUE_MESSAGE(denseIndices.insert(desc->denseIndex).second, desc->semanticName);
        TEST_ASSERT_TRUE_MESSAGE(semanticNames.insert(desc->semanticName).second, desc->semanticName);

        // Range invariants
        TEST_ASSERT_TRUE(desc->minValue <= desc->defaultValue);
        TEST_ASSERT_TRUE(desc->defaultValue <= desc->maxValue);

        // Direct lookups match
        TEST_ASSERT_EQUAL_PTR(desc, ParameterRegistry::getByName(desc->semanticName));
        TEST_ASSERT_EQUAL_PTR(desc, ParameterRegistry::getByName(desc->key));
        TEST_ASSERT_EQUAL_PTR(desc, ParameterRegistry::getByWireId(desc->wireId));

        // Enum contract verification
        if (desc->type == ParamType::Enum) {
            int maxVal = static_cast<int>(desc->maxValue);
            TEST_ASSERT_TRUE(maxVal >= 1);
            for (int v = 0; v <= maxVal; ++v) {
                const char* enumStr = LibrarySerializer::serializeEnumString(desc, v);
                TEST_ASSERT_NOT_NULL_MESSAGE(enumStr, desc->semanticName);
                int parsed = LibrarySerializer::parseEnumString(desc, enumStr);
                TEST_ASSERT_EQUAL_INT_MESSAGE(v, parsed, desc->semanticName);
            }
            // Out of bounds enum must fail serialization
            TEST_ASSERT_NULL(LibrarySerializer::serializeEnumString(desc, -1));
            TEST_ASSERT_NULL(LibrarySerializer::serializeEnumString(desc, maxVal + 1));
            TEST_ASSERT_NULL(LibrarySerializer::serializeEnumString(desc, 999));
        }
    }
}

void test_web_contract_parity(void) {
    std::ifstream ifs("schemas/voxp4-parameters-v1.json");
    TEST_ASSERT_TRUE_MESSAGE(ifs.is_open(), "Could not open schemas/voxp4-parameters-v1.json");

    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string jsonStr = ss.str();

    DynamicJsonDocument doc(65536);
    DeserializationError err = deserializeJson(doc, jsonStr);
    TEST_ASSERT_FALSE_MESSAGE(err, err.c_str());

    TEST_ASSERT_EQUAL_INT(1, doc["contractVersion"].as<int>());
    TEST_ASSERT_EQUAL_INT(71, doc["parameterCount"].as<int>());

    JsonArray paramsArr = doc["parameters"].as<JsonArray>();
    TEST_ASSERT_EQUAL_UINT(71, paramsArr.size());

    for (JsonObject p : paramsArr) {
        const char* name = p["name"];
        const char* key = p["key"];
        const char* ptype = p["type"];
        float pmin = p["min"];
        float pmax = p["max"];
        float pdef = p["default"];

        const ParamDescriptor* desc = ParameterRegistry::getByName(name);
        TEST_ASSERT_NOT_NULL_MESSAGE(desc, name);
        TEST_ASSERT_EQUAL_STRING(name, desc->semanticName);
        TEST_ASSERT_EQUAL_STRING(key, desc->key);
        TEST_ASSERT_EQUAL_FLOAT(pmin, desc->minValue);
        TEST_ASSERT_EQUAL_FLOAT(pmax, desc->maxValue);
        TEST_ASSERT_EQUAL_FLOAT(pdef, desc->defaultValue);

        if (std::strcmp(ptype, "enum") == 0) {
            TEST_ASSERT_EQUAL(ParamType::Enum, desc->type);
            TEST_ASSERT_TRUE(p.containsKey("values"));
            JsonArray values = p["values"].as<JsonArray>();
            int maxVal = static_cast<int>(desc->maxValue);
            TEST_ASSERT_EQUAL_INT(maxVal + 1, values.size());

            for (size_t vIdx = 0; vIdx < values.size(); ++vIdx) {
                const char* expectedLabel = values[vIdx];
                const char* actualLabel = LibrarySerializer::serializeEnumString(desc, static_cast<int>(vIdx));
                TEST_ASSERT_NOT_NULL(actualLabel);
                TEST_ASSERT_EQUAL_STRING(expectedLabel, actualLabel);
            }
        }
    }
}

void test_compliance_fixtures_suite(void) {
    std::string err;

    // Valid fixtures
    TEST_ASSERT_TRUE_MESSAGE(loadAndValidateFixture("tests/fixtures/compliance/valid_minimal_library.json", err), err.c_str());
    TEST_ASSERT_TRUE_MESSAGE(loadAndValidateFixture("tests/fixtures/compliance/valid_full_library.json", err), err.c_str());

    // Invalid version fixtures
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_future_format_version.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_format_version_zero.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_future_schema_version.json", err));

    // Invalid referential and uniqueness fixtures
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_duplicate_id.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_missing_preset_reference.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_missing_scene_reference.json", err));

    // Invalid parameter fixtures
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_unknown_parameter.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_parameter_type.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_parameter_range.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_enum.json", err));

    // Invalid collection limit fixtures
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_too_many_scenes.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_too_many_subscenes.json", err));
    TEST_ASSERT_FALSE(loadAndValidateFixture("tests/fixtures/compliance/invalid_too_many_setlist_entries.json", err));
}

void test_demo_setlist_canary(void) {
    std::string err;
    TEST_ASSERT_TRUE_MESSAGE(loadAndValidateFixture("examples/demo_setlist.voxp4.json", err), err.c_str());

    // Test that the canary can be executed by PerformanceSession
    std::ifstream ifs("examples/demo_setlist.voxp4.json");
    std::stringstream ss;
    ss << ifs.rdbuf();
    Library canaryLib;
    TEST_ASSERT_TRUE(LibrarySerializer::deserializeJson(ss.str(), canaryLib, err));

    PerformanceSession session(&canaryLib);
    TEST_ASSERT_NOT_NULL(session.getActiveSetlist());
    TEST_ASSERT_NOT_NULL(session.getActiveScene());
    TEST_ASSERT_NOT_NULL(session.getActiveSubscene());

    const ResolvedState& state = session.getResolvedState();
    const ParamDescriptor* dBpm = ParameterRegistry::getByName("TempoBpm");
    TEST_ASSERT_EQUAL_FLOAT(54.0f, state.get(dBpm->denseIndex).asFloat());
}

void test_serialization_determinism(void) {
    Library lib = LibraryStorage::createFactoryLibrary();

    std::string json1, json2, json3;
    TEST_ASSERT_TRUE(LibrarySerializer::serializeJson(lib, json1, true));
    TEST_ASSERT_TRUE(LibrarySerializer::serializeJson(lib, json2, true));
    TEST_ASSERT_EQUAL_STRING(json1.c_str(), json2.c_str());

    Library lib2;
    std::string err;
    TEST_ASSERT_TRUE(LibrarySerializer::deserializeJson(json1, lib2, err));
    TEST_ASSERT_TRUE(LibrarySerializer::serializeJson(lib2, json3, true));
    TEST_ASSERT_EQUAL_STRING(json1.c_str(), json3.c_str());
}

void test_atomic_import_integrity(void) {
    Library original = LibraryStorage::createFactoryLibrary();
    const char* testPath = "test_atomic_storage.json";
    TEST_ASSERT_TRUE(LibraryStorage::saveLibraryAtomic(original, testPath));

    // Corrupted JSON import attempt
    Library importedLib;
    std::string err;
    std::string corruptJson = "{ \"format\": \"voxp4-library\", TRUNCATED...";
    bool ok = LibraryStorage::importLibraryAtomic(corruptJson, importedLib, err, testPath);
    TEST_ASSERT_FALSE(ok);

    // Verify original file is still valid and untouched
    Library checkLib;
    TEST_ASSERT_TRUE(LibraryStorage::loadLibrary(checkLib, testPath));
    TEST_ASSERT_EQUAL_STRING(original.libraryId.c_str(), checkLib.libraryId.c_str());

    // Cleanup
    std::remove(testPath);
    std::remove("library.voxp4.tmp.json");
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_parameter_value_types);
    RUN_TEST(test_compact_param_set_operations);
    RUN_TEST(test_parameter_registry_lookups);
    RUN_TEST(test_state_resolver_5_tier_inheritance);
    RUN_TEST(test_sparse_override_isolation);
    RUN_TEST(test_parameter_diff_exact_deltas);
    RUN_TEST(test_performance_session_navigation);
    RUN_TEST(test_temporary_edits_dirty_and_revert);
    RUN_TEST(test_commit_temporary_edits);
    RUN_TEST(test_library_validator);
    RUN_TEST(test_web_editor_golden_fixture_and_roundtrip);
    RUN_TEST(test_71_parameter_contract_audit);
    RUN_TEST(test_web_contract_parity);
    RUN_TEST(test_compliance_fixtures_suite);
    RUN_TEST(test_demo_setlist_canary);
    RUN_TEST(test_serialization_determinism);
    RUN_TEST(test_atomic_import_integrity);
    return UNITY_END();
}
