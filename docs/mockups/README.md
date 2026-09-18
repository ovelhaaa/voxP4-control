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
- Nenhuma área grande pintada de cor saturada; efeitos ativos usam apenas LED e
  borda/fita de destaque.
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
- Tab ativa: texto off-white + linha superior laranja de 3px. Sem preenchimento
  forte na tab ativa.

#### 2. Header (24px)
- Preset atual tem prioridade visual.
- Indicador de link: turquesa `● LINK` conectado, discreto quando desconectado.

#### 3. Pitch (40px)
- Nota musical em destaque com turquesa (`A3`).
- Frequência (`220.1 Hz`) e estado `VOICED`/`UNVOICED` à direita.

#### 4. VuMeter Horizontal (16px por linha)
- Trilha escura, preenchimento turquesa.
- Zona de aviso laranja e clip vermelho.
- Valor numérico discreto à direita.

#### 5. Effect Cards (72×46)
- Estado ON: LED laranja + borda laranja; OFF: borda separadora + LED apagado.
- Nunca preenche o card inteiro de cor.

#### 6. Footswitch Summary (32px)
- FS1/FS2 com ação resumida; pressionado usa laranja.

#### 7. Parameter Row (Effect Edit)
- Linha rotulada com leitura de valor, slider horizontal laranja e knob.
- Seletor de enum em segmentos (`L / C / R`).
- Lista de parâmetros rolável, preparada para múltiplas vozes de Harmony.

### Mockups Disponíveis

| Arquivo | Descrição |
|---------|-----------|
| `boot_screen.svg` | Splash de boot (`VOXP4 / CONTROL`) |
| `performance_screen.svg` | Tela principal ao vivo |
| `fx_chain_screen.svg` | Rack de módulos + ações globais |
| `effect_edit_screen.svg` | Editor de parâmetros |
| `presets_screen.svg` | Browser de presets |
| `footswitch_screen.svg` | Configuração de FS1/FS2 |
| `settings_screen.svg` | Menu de settings |
| `system_screen.svg` | Diagnóstico (rolável) |

### Hierarquia da Performance Screen

```
┌────────────────────────────────────┐
│ P03 LEAD AIR              ● LINK    │  Header 24
├────────────────────────────────────┤
│ A3                    220.1 Hz      │  Pitch 40
│                        VOICED       │
├────────────────────────────────────┤
│ IN  [██████████░░░░]         -12     │  Meters 34
│ OUT [████████████░░]          -6     │
├────────────────────────────────────┤
│ HARMONY  REVERB  DELAY  LIMITER     │  Effects 46
│   ●ON      ●ON     OFF     ●ON      │
├────────────────────────────────────┤
│ FS1  HARMONY        FS2  REVERB     │  FS 32
├────────────────────────────────────┤
│   PERF   │  FX  │ PRESET │  SET     │  Nav 36
└────────────────────────────────────┘
```

### Critérios de Aceitação Visual

- Parece processador vocal musical, não demo LVGL genérica.
- Hierarquia clara em menos de 1 segundo.
- Laranja não domina a tela; turquesa sinaliza áudio.
- Efeitos ON visíveis sem grandes áreas coloridas.
- Nada ultrapassa 320×240.
- Contraste adequado para palco.
- Consistente em todas as telas; sem paleta antiga.

### Validação

- **Build host verificado**: PlatformIO `esp32-cyd` (`espressif32@6.5.0`),
  `SUCCESS` (RAM ~30.6%, Flash ~48.2%).
- **Validação em hardware**: pendente. Ajustes finais de contraste, calibração
  de toque e legibilidade sob luz de palco precisam de teste no CYD real.

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
