# VoxP4 Control Surface Mockups

## Design System v3 - Dark Industrial / Warm Orange / Turquoise Audio

Esta coleção de mockups SVG representa a interface gráfica do **VoxP4-CYD Control Surface**, um controlador para processador vocal profissional baseado em ESP32 CYD (320×240 pixels, touchscreen resistivo).

Todos os arquivos usam `viewBox="0 0 320 240"` e representam a UI real final
(implementada em `src/ui/`), e não telas aspiracionais.

### Identidade visual

- Base quase preta / charcoal, superfícies grafite.
- Acento principal laranja queimado, reservado para **seleção, efeito ativo,
  botão primário, toggle ativo, slider e foco**.
- Turquesa reservado para **áudio ao vivo**: pitch, meters, telemetria.
- Nenhuma área grande pintada de cor saturada; efeitos ativos usam apenas rail,
  LED e texto mais forte.
- Estética de hardware musical de palco, não de dashboard genérico.

### Paleta de Cores (Design Tokens em `src/ui/UiTheme.h`)

| Token | Valor | Uso |
|-------|-------|-----|
| `COLOR_BG` | `#090A0E` | Background principal |
| `COLOR_SURFACE` | `#14151B` | Cards e superfícies |
| `COLOR_SURFACE_ELEV` | `#1C1D24` | Superfícies elevadas |
| `COLOR_HEADER` | `#101116` | Arriba / header |
| `COLOR_NAV` | `#101116` | Barra de navegação |
| `COLOR_PANEL` | `#202128` | Painéis secundários / trilhas |
| `COLOR_BORDER` | `#34343C` | Bordas |
| `COLOR_SEPARATOR` | `#292A30` | Separadores discretos |
| `COLOR_TEXT_PRIMARY` | `#F0EDE5` | Texto principal |
| `COLOR_TEXT_SECONDARY` | `#B1ACA3` | Texto secundário |
| `COLOR_TEXT_MUTED` | `#716E69` | Texto auxiliar / inativo |
| `COLOR_TEXT_FAINT` | `#4A4947` | Texto muito apagado |
| `COLOR_ACCENT` | `#F45126` | Acento laranja (seleção/active) |
| `COLOR_ACCENT_BRIGHT` | `#FF6030` | Laranja claro (valores em foco) |
| `COLOR_ACCENT_DARK` | `#B83A1C` | Laranja escuro (chips/ações) |
| `COLOR_AUDIO` | `#20D6C7` | Turquesa de áudio |
| `COLOR_AUDIO_BRIGHT` | `#55EFE2` | Turquesa claro (nota/level) |
| `COLOR_AUDIO_DARK` | `#148E87` | Turquesa escuro (link/outline) |
| `COLOR_WARNING` | `#E6A63A` | Aviso (meter warning) |
| `COLOR_ERROR` | `#E65050` | Erro / clip |
| `COLOR_DISABLED` | `#414147` | Desabilitado / LED apagado |

### Tipografia

Fontes Montserrat já compiladas no firmware (`10/12/14/16`).

| Tam. | Uso | Exemplo |
|------|-----|---------|
| 16px | Hero / nota de pitch, títulos de destaque | `A3`, `FS1` |
| 14px | Títulos e valores principais | `HARMONY`, `LEAD AIR` |
| 12px | Controles e valores | `MOMENTARY`, `+3rd` |
| 10px | Metadata e labels | `FS1`, `IN`, `VOXLINK` |

Caixa alta é usada em labels, categorias e pequenas identificações; não em todo
o conteúdo.

### Layout

- Margem externa: 4–6px.
- Gaps: 2px (micro), 4px (comum), 6px (seções).
- Corner radius: 3px (controles), 5px (cards).
- Área de conteúdo: 320×204; barra de navegação: 320×36.
- Targets de toque importantes: ~36–44px.

### Componentes Principais

#### 1. Navigation Bar (36px)
- 4 tabs: **PERF | FX | PRESET | SET**
- Tab inativa: texto cinza (`COLOR_TEXT_MUTED`).
- Tab ativa: texto off-white + linha superior laranja de 2px. Sem preenchimento
  forte na tab ativa; pressionado usa superfície levemente elevada.

#### 2. Header (24px)
- Preset atual tem prioridade visual.
- Indicador de link: turquesa `● LINK` conectado, discreto quando desconectado.

#### 3. Pitch (40px)
- Nota musical em destaque com turquesa (`A3`).
- Frequência (`220.1 Hz`) e estado `VOICED`/`UNVOICED` à direita.

#### 4. VuMeter Horizontal (16px por linha)
- Trilha escura; a barra permanece turquesa em todos os níveis.
- Warning/clip são sinalizados por um pequeno marcador à direita (LED de clip
  com hold curto), não recolorindo a barra inteira.
- Valor numérico discreto à direita.

#### 5. Effect Cards (58×46, 7 módulos, scroll horizontal)
- Sete indicadores: **GATE · COMP · HARM · DRIVE · MOD · DLY · REV**.
- A linha rola horizontalmente com snap; o 6º card aparece cortado na borda
  (dica de scroll). Não se espremem 7 cards na largura de 320.
- Estado ON: LED laranja + rail de accent no topo, totalmente dentro do card
  (clip corner) e alinhado à linguagem dos módulos da FX Chain.
- A borda do card permanece neutra (`COLOR_SEPARATOR`) em ON e OFF.
- Nunca preenche o card inteiro de cor.

#### 6. FX Chain Modules (64×84, 7 módulos, scroll horizontal)
- Cadeia: **GATE → COMPRESSOR → HARMONY → DRIVE → MODULATION → DELAY → REVERB**.
- A borda permanece sempre neutra (`COLOR_SEPARATOR`), evitando a leitura de
  "caixas contornadas".
- ON é comunicado por rail laranja no topo + LED + texto mais forte.
- Valor principal domina (`FONT_EMPHASIS`); metadata discreta embaixo.

#### 7. Footswitch Summary (32px)
- FS1/FS2 com ação resumida; pressionado usa laranja.

#### 8. Effect Editor (M7, data-driven, Basic/Advanced)
- O body é reconstruído a partir de descritores por efeito; header/back/enable
  são persistentes.
- Controles: `Slider`, `Toggle`, `Segmented` (2–4 opções) e `Stepper`
  (`[-] valor [+]`) para enums (KEY/SCALE/SUBDIVISION).
- Seções **BASIC** (aberta) e **ADVANCED** (fechada por padrão); o usuário abre
  o Advanced explicitamente.
- Editores: GATE, COMPRESSOR, HARMONY, DRIVE, MODULATION, DELAY, REVERB.
- Harmony muda de layout conforme MODE (FIXED / DIATONIC / MIDI); uma única voz
  (sem `VOICE 1`). Advanced inclui VOICE, FORMANT, DYNAMICS, DRY ALIGN,
  MIDI RANGE e HARM LIMITER.
- Delay/Chorus: `TEMPO SYNC` alterna entre ms e subdivisions canônicas (1/1 …
  1/16T), usando o `TempoBpm` global.
- O antigo card `LIMITER` foi removido: `harmony.limiter.*` agora é
  HARMONY → ADVANCED → HARM LIMITER. O limiter master vive em `MASTER`.
- Sem placeholders falsos: nada de `PLATE`, `KEY AUTO` ou `REVERB FREEZE`.

#### 9. MASTER / ROUTING (SET → MASTER)
- Área global fora da Performance: `TEMPO` (BPM), `LIMITER CEILING`,
  `MUTE DRY`, `SPATIAL ROUTING` (Parallel / DelayIntoReverb) e
  `SPATIAL SOURCE` (Input / PostDynamics / PostHarmony).
- Sem enable de limiter master (esse parâmetro não existe); apenas o ceiling.
- Afordância `TAP TEMPO` no header.

#### 10. Tempo
- O BPM compacto aparece no header da Performance e abre o MASTER ao toque.
- `TAP TEMPO` (footswitch ou MASTER) calcula BPM no CYD e envia `TempoBpm` como
  parâmetro normal.

### Interação

- Todos os controles clicáveis têm estado `LV_STATE_PRESSED` imediato e barato
  (fundo levemente elevado e/ou borda mais forte), sem animação, sombra ou scale.
- A seleção de preset usa poucos sinais: superfície elevada + rail lateral
  laranja + nome off-white (sem borda e sem texto laranja simultâneos).
- O preset atual usa número em laranja, nome off-white e label `CURRENT`
  discreto.

### Mockups Disponíveis

| Arquivo | Descrição |
|---------|-----------|
| `boot_screen.svg` | Splash de boot (`VOXP4 / CONTROL`) |
| `performance_screen.svg` | Tela principal ao vivo (7 indicadores + BPM) |
| `fx_chain_screen.svg` | Rack de 7 módulos com scroll + ações globais |
| `effect_edit_screen.svg` | Editor HARMONY (MODE=DIATONIC, BASIC) |
| `effect_edit_harmony_advanced_screen.svg` | HARMONY → ADVANCED (dry align, harm limiter) |
| `effect_edit_gate_screen.svg` | Editor GATE |
| `effect_edit_compressor_screen.svg` | Editor COMPRESSOR |
| `effect_edit_drive_screen.svg` | Editor DRIVE |
| `effect_edit_modulation_screen.svg` | Editor MODULATION (CHORUS) |
| `effect_edit_delay_screen.svg` | Editor DELAY (TEMPO SYNC + subdivisions) |
| `effect_edit_reverb_screen.svg` | Editor REVERB |
| `master_screen.svg` | MASTER / ROUTING (tempo, ceiling, routing) |
| `presets_screen.svg` | Browser de presets |
| `footswitch_screen.svg` | Configuração/edição de FS1/FS2 |
| `settings_screen.svg` | Menu de settings (inclui MASTER / ROUTING) |
| `system_screen.svg` | Diagnóstico (rolável) |

### Hierarquia da Performance Screen

```
┌────────────────────────────────────┐
│ P03 LEAD AIR        120 BPM  ● LINK │  Header 24
├────────────────────────────────────┤
│ A3                    220.1 Hz      │  Pitch 40
│                        VOICED       │
├────────────────────────────────────┤
│ IN  [██████████░░░░]         -12     │  Meters 34
│ OUT [████████████░░]          -6     │
├────────────────────────────────────┤
│ GATE COMP HARM DRIVE MOD DLY ▸REV   │  Effects 46 (scroll h)
│  ●ON  ●ON  OFF  OFF  OFF  ●ON  ●ON  │
├────────────────────────────────────┤
│ FS1  HARMONY        FS2  REVERB     │  FS 32
├────────────────────────────────────┤
│   PERF   │  FX  │ PRESET │  SET     │  Nav 36
└────────────────────────────────────┘
```

O rack de efeitos rola horizontalmente (snap); `DLY`/`REV` aparecem ao
arrastar. O BPM no header abre o MASTER ao toque.

### Critérios de Aceitação Visual

- Parece processador vocal musical, não demo LVGL genérica.
- Hierarquia clara em menos de 1 segundo.
- Laranja não domina a tela; turquesa sinaliza áudio.
- Efeitos ON visíveis sem grandes áreas coloridas.
- Nada ultrapassa 320×240.
- Contraste adequado para palco.
- Consistente em todas as telas; sem paleta antiga.

### Validação

- **Build M7**: PlatformIO `esp32-cyd` (`espressif32@6.5.0`) `SUCCESS`
  (RAM 18.2%, Flash 57.6%).
- **Testes host M7**: `pio test -e native` 82/82 PASS, incluindo o teste de
  cobertura que exige que todos os 71 parâmetros do contrato tenham um único
  lugar na UI.
- **Cobertura da UI**: `python scripts/audit_ui_coverage.py --check` — 71/71
  parâmetros colocados, 0 ausentes. Tabela completa em
  [`../m7_param_ui_coverage.md`](../m7_param_ui_coverage.md).
- **Validação em hardware**: pendente. Ajustes finais de contraste, calibração
  de toque, scroll horizontal do rack e legibilidade sob luz de palco precisam
  de teste no CYD real.

### Notas de Implementação

- Cores e estilos centralizados em `src/ui/UiTheme.h` e `UiTheme.cpp`; as telas
  não usam cores hardcoded.
- SVGs são representações fiéis do renderizável em LVGL no ESP32.
- Sem gradientes complexos, sombras ou blur.
- Atualizações de telemetria não recriam objetos LVGL.
- Screen manager com containers persistentes (show/hide).

---

**VoxP4 Control Surface Design System v3**
*Dark Industrial + Warm Orange + Turquoise Audio Feedback*
