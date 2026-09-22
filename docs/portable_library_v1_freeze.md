# Milestone Report — Portable Library V1 Contract Freeze & Web-Editor Readiness Audit

**Project**: `ovelhaaa/voxp4-control`  
**Milestone**: Portable Library V1 Contract Freeze & Web-Editor Readiness Audit  
**Date**: September 2026  
**Status**: AUDIT COMPLETE — RECOMMENDATION: **FREEZE V1**

---

## Executive Summary

This milestone audited and froze the public **Portable Library V1** contract (`.voxp4.json`). All data definitions, serialization logic, validation rules, and collection limits have been audited against the canonical VoxP4 DSP source files.

Key audit fixes include:
1. **Enum Alignment**: Corrected inverted `ChorusMode` mappings and misordered `HarmonyScale` enum tables in `LibrarySerializer` against canonical DSP headers (`chorus.h` and `scale.h`).
2. **Strict Enum Serialization**: Eliminated fallback to raw integer ordinals on known enums. Serialization now strictly outputs canonical human-readable strings, and invalid ordinals fail serialization immediately.
3. **Strict Versioning**: Fixed `formatVersion == 1` and `schemaVersion == 1` with rejection of unsupported future versions or zero versions.
4. **Limits Harmonization**: Frozen production limits at 64 Presets, 64 Scenes, 16 Subscenes/Scene, 16 Setlists, 64 Entries/Setlist, and 128 KB max file size.
5. **JSON Schema Hardening**: Added `additionalProperties: false` to all structural envelopes, string length bounds (`1..64`), and array `maxItems`.
6. **Exportable Parameter Contract**: Generated [`schemas/voxp4-parameters-v1.json`](file:///c:/progs/VoxP4/voxP4-control/schemas/voxp4-parameters-v1.json) directly from `voxlink_params.json` with zero wire-protocol leakage.
7. **Comprehensive Compliance Suite**: Created 15 compliance fixtures in `tests/fixtures/compliance/` covering valid scenarios and invalid boundary conditions.
8. **Automated Parity & Reproducibility**: Added automated tests in Unity verifying 100% parity between C++ `ParameterRegistry` and `voxp4-parameters-v1.json`, plus a CI reproducibility checker (`scripts/check_contract_reproducibility.py`).

**Results**: 74 / 74 tests pass (100%), and the firmware builds cleanly on `esp32-cyd` with 17.9% RAM and 56.8% Flash utilization.

---

## Objective Answers to the 22 Milestone Questions

### 1. Qual é a fonte de verdade do Portable Library V1?
A fonte de verdade pública e agnóstica de linguagem é a especificação [`docs/portable_library_format_v1.md`](file:///c:/progs/VoxP4/voxP4-control/docs/portable_library_format_v1.md), formalizada pelo schema estrutural [`schemas/voxp4-library-v1.schema.json`](file:///c:/progs/VoxP4/voxP4-control/schemas/voxp4-library-v1.schema.json) e pelo catálogo de parâmetros [`schemas/voxp4-parameters-v1.json`](file:///c:/progs/VoxP4/voxP4-control/schemas/voxp4-parameters-v1.json).

### 2. `formatVersion=1` está efetivamente congelado?
**Sim**. O valor de `formatVersion` deve ser exatamente `1`. Qualquer arquivo com `formatVersion != 1` (inclusive `0` ou `2+`) é rejeitado atomicamente pelo `LibraryValidator` e pelo JSON Schema.

### 3. Qual é a semântica de `schemaVersion`?
`schemaVersion` define a revisão compatível dentro do mesmo `formatVersion`. Para esta baseline congelada, `schemaVersion == 1` é estritamente exigido pelo firmware V1 (`kSupportedSchemaVersion = 1`). Versões futuras maiores que 1 serão suportadas apenas quando houver revisões formais documentadas e testadas.

### 4. Quais são os limites finais de Presets/Scenes/Subscenes/Setlists/Entries?
- **Presets**: máximo de 64
- **Scenes**: máximo de 64
- **Subscenes por Scene**: máximo de 16
- **Setlists**: máximo de 16
- **Entries por Setlist**: máximo de 64
- **String Length**: 1 a 64 caracteres (nomes e IDs)
- **Tamanho Máximo do Arquivo**: 131.072 bytes (128 KB)

### 5. Schema e runtime concordam nesses limites?
**Sim**. `schemas/voxp4-library-v1.schema.json` e `LibraryValidator` utilizam exatamente os mesmos valores (`maxItems: 64, 64, 16, 16, 64` e `kMaxPresets=64, kMaxScenes=64, kMaxSubscenesPerScene=16, kMaxSetlists=16, kMaxEntriesPerSetlist=64`).

### 6. Quais validações são estruturais?
Validações de **Nível 1 (JSON Schema)**:
- Sintaxe JSON válida;
- Campos obrigatórios do envelope (`format`, `formatVersion`, `libraryId`, `name`);
- Versões exatas (`formatVersion: 1`, `schemaVersion: 1`);
- Tipos de dados (números, booleans, strings, arrays de objetos);
- Limites de tamanho de coleções (`maxItems`);
- Comprimento de strings (`minLength: 1`, `maxLength: 64`);
- Rejeição de campos desconhecidos nos objetos estruturais (`additionalProperties: false`).

### 7. Quais são semânticas?
Validações de **Nível 2 (Firmware / ParameterRegistry / LibraryValidator)**:
- Unicidade de IDs (Presets, Scenes, Subscenes locais, Setlists, SetlistEntries);
- Integridade referencial (`scene.basePresetId` deve existir em `presets[]`; `entry.sceneId` deve existir em `scenes[]`);
- Existência do parâmetro no catálogo de 71 parâmetros;
- Pertencimento do valor numérico ao intervalo $[min, max]$;
- Rejeição de valores `NaN` ou infinitos;
- Validação estrita de enums (rejeição de strings de enum inexistentes).

### 8. Unknown parameters são rejeitados, avisados ou ignorados?
**Rejeitados**. O CYD adota a política de **Strict Import**. Se um arquivo contiver qualquer chave desconhecida em `parameters`, a desserialização falha com erro explícito (`Unknown parameter: '<nome>'`), impedindo que uma biblioteca com parâmetros ausentes seja ativada em performance ao vivo.

### 9. Como versões futuras são tratadas?
Arquivos com `formatVersion > 1` são rejeitados de forma transacional e atômica. A biblioteca ativa atual permanece intacta no storage, sem sofrer overwrite ou corrupção.

### 10. Os 71 parâmetros atuais possuem representação portátil completa?
**Sim**. O teste automatizado `test_71_parameter_contract_audit` percorre todos os 71 parâmetros e comprova que cada um possui nome semântico válido, chave estável, tipo, valor padrão, intervalo $[min, max]$ e metadados de enum verificados.

### 11. Há nomes, wire IDs ou dense indexes duplicados?
**Não**. O teste de auditoria comprova 0 colisões em nomes semânticos, 0 colisões em wire IDs e mapeamento bijetivo contínuo em dense indexes ($0..70$).

### 12. Enums fazem round-trip por labels semânticos?
**Sim**. Todos os 11 tipos de enum (`HarmonyMode`, `HarmonyKey`, `HarmonyScale`, `HarmonyVoice1NonScalePolicy`, `DelayLeftSubdivision`, `DelayRightSubdivision`, `OutputSpatialRouting`, `OutputSpatialSource`, `ChorusMode`, `ChorusSubdivision`, `DriveMode`) são serializados como strings canônicas em PascalCase e desserializados com tolerância a maiúsculas/minúsculas e aliases musicais (e.g. `"Db"` para `"C#"`, `"Overdrive"` para `"Crunch"`).

### 13. O formato depende de enum ordinals?
**Não**. O arquivo portátil utiliza exclusivamente labels textuais públicos.

### 14. O formato depende de wire IDs?
**Não**. O arquivo portátil não contém nenhum ID hex binário do protocolo VoxLink (`0x0100`, etc.). As chaves utilizam nomes semânticos (`HarmonyEnable`, `MicroshiftLeftCents`, etc.).

### 15. O formato depende de dense indexes?
**Não**. Os índices densos $0..70$ são detalhes de implementação interna em C++ do CYD e nunca são expostos no JSON.

### 16. O fixture de autoria externa passa?
**Sim**. `tests/fixtures/library_v1.json` (autoria simulada do Web Editor) é validado, desserializado e resolvido com 100% de sucesso.

### 17. O demo setlist passa pela pipeline real?
**Sim**. `examples/demo_setlist.voxp4.json` é testado como canary em `test_demo_setlist_canary`, sendo carregado, validado e instanciado em uma `PerformanceSession` ativa.

### 18. O round-trip é semanticamente lossless?
**Sim**. `JSON -> Library -> JSON -> Library` preserva com exatidão todos os IDs, nomes, metadados, valores numéricos, flags booleanas e enums.

### 19. O serializer é determinístico?
**Sim**. O teste `test_serialization_determinism` comprova que invocar `serializeJson()` múltiplas vezes sobre o mesmo modelo gera strings idênticas byte-a-byte com ordenação canônica de chaves.

### 20. Uma importação inválida preserva a biblioteca anterior?
**Sim**. O teste `test_atomic_import_integrity` demonstra que uma tentativa de importação com JSON corrompido ou semanticamente inválido é abortada e a biblioteca anterior no storage permanece 100% intacta.

### 21. O contrato exportável de parâmetros pode ser consumido por JavaScript sem conhecer C++?
**Sim**. O arquivo [`schemas/voxp4-parameters-v1.json`](file:///c:/progs/VoxP4/voxP4-control/schemas/voxp4-parameters-v1.json) contém o catálogo completo dos 71 parâmetros em JSON puro: nomes, chaves, displays, grupos, tipos, mínimos, máximos, valores padrão, passos, unidades e arrays de strings para cada enum, sem dependências de C++ ou LVGL.

### 22. Há alguma decisão restante que impeça iniciar um Web Editor independente?
**Nenhuma**. O formato portátil, o JSON Schema e o catálogo de parâmetros estão formalizados, congelados e cobertos por testes automatizados.

---

## Resultados da Verificação

### Testes Automatizados Nativos (PlatformIO Unity)
```pwsh
& C:\.platformio\penv\Scripts\pio.exe test -e native
```
- `test_param_model`: 29 passed
- `test_scene_runtime`: 17 passed
- `test_voxlink`: 28 passed
- **Total**: **74 / 74 PASSED (100%)** em 7.21 segundos.

### Reproducibilidade do Gerador de Parâmetros
```pwsh
python scripts/check_contract_reproducibility.py
```
- Status: **OK (zero diff)** entre source-of-truth e arquivos gerados.

### Compilação de Firmware ESP32-CYD
```pwsh
& C:\.platformio\penv\Scripts\pio.exe run -e esp32-cyd
```
- Status: **SUCCESS**
- **RAM**: 58.648 bytes / 327.680 bytes (17.9% em uso — mais de 240 KB livres).
- **Flash**: 744.249 bytes / 1.310.720 bytes (56.8% em uso).

---

## Recomendação Técnica Final

> **RECOMENDAÇÃO: `FREEZE V1`**
>
> O contrato público da Portable Library V1 está completamente auditado, harmonizado, congelado e qualificado. O desenvolvimento do futuro Web Editor independente pode ser iniciado com total segurança e previsibilidade.
