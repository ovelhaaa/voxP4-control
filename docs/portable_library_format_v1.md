# VoxP4 Portable Library Format — Version 1 (V1) Specification

## 1. Overview and Purpose

The **VoxP4 Portable Library Format (V1)** is the public, language-agnostic data contract for storing and exchanging musical performance configurations within the VoxP4 ecosystem.

This specification enables external authoring tools (such as future Web Editors, desktop library managers, mobile companion apps, and command-line scripts) to create, inspect, edit, version, and validate VoxP4 project files that can be directly imported and executed by the `voxP4-control` physical surface and any future runtimes.

### Core Architectural Invariant
> **The portable library format represents the musical and performance state of the VoxP4 vocal processor.**
> It is strictly decoupled from C++ implementation classes, LVGL widgets, GUI screen layout, and raw VoxLink wire protocol packets.

---

## 2. File Identification & Encoding

- **Format Identifier**: `voxp4-library`
- **Format Version**: `1`
- **Schema Version**: `1`
- **Character Encoding**: UTF-8 without Byte Order Mark (BOM).
- **File Extensions**: `.voxp4.json` (recommended) or `.json`.
- **MIME Type**: `application/vnd.voxp4.library+json` or `application/json`.
- **Determinism**: Serializers SHOULD emit fields in standard canonical order with 2-space indentation to facilitate Git diffs, human readability, and deterministic round-tripping.

---

## 3. Object Identification & Identity Rules

All top-level entities (`Preset`, `Scene`, `Subscene`, `Setlist`, `SetlistEntry`) possess a persistent string identifier (`id`):
- **Immutability**: An `id` is an immutable, opaque string. Renaming an object's display `name` MUST NOT change its `id`.
- **Uniqueness**:
  - `Preset` IDs must be unique across `presets[]`.
  - `Scene` IDs must be unique across `scenes[]`.
  - `Subscene` IDs must be unique within their parent `Scene.subscenes[]`.
  - `Setlist` IDs must be unique across `setlists[]`.
- **Format**:
  - Semantic slug format is recommended: lowercase alphanumeric words separated by hyphens (e.g. `"preset-wide-vocal"`, `"scene-creep"`, `"subscene-chorus-lead"`).
  - Standard UUID strings (e.g. `"a8098c1a-f86e-11da-bd1a-00112444be1e"`) are fully valid and supported.
  - Array indices MUST NOT be used as persistent identities.

---

## 4. Domain Model Schema

A portable library file is a single JSON object structured as follows:

```json
{
  "format": "voxp4-library",
  "formatVersion": 1,
  "schemaVersion": 1,
  "libraryId": "rig-tour-2026",
  "name": "Live Tour Rig 2026",
  "presets": [ ... ],
  "scenes": [ ... ],
  "setlists": [ ... ]
}
```

### 4.1. Top-Level Fields

| Field | Type | Required | Description |
| :--- | :--- | :--- | :--- |
| `format` | string | Yes | Must be `"voxp4-library"`. |
| `formatVersion` | integer | Yes | Must be `1` for this specification. |
| `schemaVersion` | integer | Yes | Semantic schema revision (defaults to `1`). |
| `libraryId` | string | Yes | Stable, opaque identifier for the entire library. |
| `name` | string | Yes | Human-readable title of the library (max 64 chars). |
| `presets` | array | Yes | Array of reusable Preset objects. |
| `scenes` | array | Yes | Array of Scene (song/performance) objects. |
| `setlists` | array | Yes | Array of Setlist objects. |

---

### 4.2. Preset Object

A `Preset` represents a reusable sound configuration.

```json
{
  "id": "preset-wide-vocal",
  "name": "Wide Vocal",
  "parameters": {
    "HarmonyEnable": true,
    "HarmonyLevel": 0.85,
    "ModulationMode": "Microshift",
    "ModulationMix": 0.35,
    "MicroshiftLeftCents": -7.0,
    "MicroshiftRightCents": 9.0,
    "ReverbWet": 0.22
  }
}
```

- `id`: Unique string ID.
- `name`: Display name.
- `parameters`: Sparse object mapping semantic parameter names to values. Does not need to include all 71 parameters.

---

### 4.3. Scene Object

A `Scene` represents an autonomous performance unit (e.g. a song, spoken word segment, or special acoustic texture).

```json
{
  "id": "scene-song-a",
  "name": "Song A",
  "basePresetId": "preset-wide-vocal",
  "metadata": {
    "artist": "The Band",
    "notes": "Intro starts with acoustic guitar",
    "tags": "live, ballad"
  },
  "parameters": {
    "TempoBpm": 95.0,
    "HarmonyKey": "E",
    "HarmonyScale": "Major"
  },
  "subscenes": [ ... ]
}
```

- `id`: Unique string ID.
- `name`: Display name of the scene.
- `basePresetId`: (Optional) ID of a preset in `presets[]` to inherit from.
- `metadata`: (Optional) Freeform non-DSP metadata: `artist`, `notes`, `tags`.
  - **Notice**: Musical properties such as tempo, tonal key, and scale MUST NOT be duplicated in `metadata`. They exist strictly as parameters (`TempoBpm`, `HarmonyKey`, `HarmonyScale`).
- `parameters`: (Optional) Sparse overrides applied on top of the base preset.
- `subscenes`: Ordered array of `Subscene` objects.

---

### 4.4. Subscene Object

A `Subscene` represents a section or variation within a scene (e.g. Intro, Verse, Chorus, Bridge, Solo).

```json
{
  "id": "song-a-chorus",
  "name": "Chorus",
  "parameters": {
    "HarmonyEnable": true,
    "DelayEnable": true,
    "DelayWet": 0.24,
    "ReverbWet": 0.30
  }
}
```

- `id`: Unique string ID (within the scene).
- `name`: Display name (e.g. `"Verse"`, `"Chorus"`).
- `parameters`: Sparse overrides applied on top of the scene and preset.

---

### 4.5. Setlist & SetlistEntry Object

A `Setlist` defines an ordered sequence of Scenes for live performance.

```json
{
  "id": "setlist-tour-night-1",
  "name": "Tour Night 1",
  "entries": [
    {
      "id": "entry-1",
      "sceneId": "scene-song-a"
    },
    {
      "id": "entry-2",
      "sceneId": "scene-song-b"
    },
    {
      "id": "entry-3",
      "sceneId": "scene-song-a"
    }
  ]
}
```

- A scene may appear multiple times in the same setlist (e.g. an opening song re-played as an encore). Each occurrence is uniquely identified by its `id`.
- `sceneId` must reference a valid `id` in `scenes[]`.

---

## 5. Semantic Parameter Representation

Parameter keys in `"parameters"` objects MUST use stable semantic names rather than raw wire hex numbers (`0x0100`).

### 5.1. Naming Conventions
Both PascalCase and dot-notated keys are supported by the reference parser:
- PascalCase (Primary): `"HarmonyEnable"`, `"TempoBpm"`, `"ReverbWet"`, `"DelayFeedback"`, `"MicroshiftLeftCents"`.
- Dot Notation: `"harmony.enable"`, `"tempo.bpm"`, `"reverb.wet"`, `"delay.feedback"`.

### 5.2. Supported Value Types
- **Boolean**: `true` or `false`.
- **Integer**: e.g. `4`, `-7`.
- **Float**: e.g. `0.25`, `120.0`, `-18.0`.
- **Enum Values**: May be supplied either as an integer ordinal (0, 1, 2...) or as human-readable string labels:
  - `HarmonyMode`: `"Fixed"`, `"Diatonic"`, `"Midi"`
  - `HarmonyKey`: `"C"`, `"C#"`, `"Db"`, `"D"`, `"D#"`, `"Eb"`, `"E"`, `"F"`, `"F#"`, `"Gb"`, `"G"`, `"G#"`, `"Ab"`, `"A"`, `"A#"`, `"Bb"`, `"B"`
  - `HarmonyScale`: `"Major"`, `"Minor"`, `"Dorian"`, `"Phrygian"`, `"Lydian"`, `"Mixolydian"`, `"Locrian"`, `"HarmonicMinor"`, `"MelodicMinor"`, `"PentatonicMajor"`, `"PentatonicMinor"`, `"Blues"`
  - `ModulationMode`: `"Dimension"`, `"Chorus"`, `"Ensemble"`, `"Microshift"`
  - `DriveMode`: `"Warm"`, `"Crunch"`, `"Lead"`

---

## 6. Deterministic Resolution Hierarchy

State resolution is strictly hierarchical and deterministic:

```text
Firmware Defaults
        ↓
Base Preset Overrides
        ↓
Scene Overrides
        ↓
Subscene Overrides
        ↓
Temporary Performance Edits (Runtime only)
```

$$\text{ResolvedParameterState} = \text{Defaults} \oplus \text{Preset} \oplus \text{Scene} \oplus \text{Subscene} \oplus \text{TemporaryEdits}$$

Where $\oplus$ indicates that a higher tier replaces any parameter specified in lower tiers.

---

## 7. Validation & Error Handling Rules

A valid library must satisfy all the following rules:

1. **Format Identifier**: `format` must equal `"voxp4-library"`.
2. **Version Bounds**: `formatVersion` must be $\le 1$.
3. **Required Identifiers**: `libraryId` and `name` must be non-empty strings.
4. **Duplicate IDs**: No duplicate IDs within `presets[]`, `scenes[]`, `setlists[]`, or within any `subscenes[]`.
5. **Referential Integrity**:
   - Every `scene.basePresetId` must match an existing `preset.id`.
   - Every `setlist.entry.sceneId` must match an existing `scene.id`.
6. **Parameter Validation**:
   - Parameter keys must correspond to known VoxP4 parameters. Unknown parameters trigger validation warnings or errors.
   - Values must not be `NaN` or infinite.
   - Numeric values must be within the defined $[min, max]$ range of the parameter descriptor.
7. **Collection Limits**:
   - Maximum presets: 64
   - Maximum scenes: 128
   - Maximum subscenes per scene: 32
   - Maximum setlists: 32
   - Maximum entries per setlist: 64

---

## 8. Migration & Future Versioning Policy

- When new versions of the format are introduced (e.g. `formatVersion: 2`):
  - Forward-compatibility: Runtimes MUST reject files with unsupported `formatVersion > kSupportedFormatVersion` atomically, without corrupting existing valid libraries.
  - Backward-compatibility: Future runtimes will implement explicit `migrateV1ToV2()` transformation passes during import.
