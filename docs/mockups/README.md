# VoxP4 Control Surface Mockups

## Design System v2 - Professional Vocal Processor Interface

Esta coleção de mockups SVG representa a interface gráfica do **VoxP4-CYD Control Surface**, um controlador para processador vocal profissional baseado em ESP32 CYD (320×240 pixels, touchscreen resistivo).

### Princípios de Design

A interface foi projetada para:

- **Leitura instantânea** em palco sob iluminação variável
- **Hierarquia visual clara** com informação musical prioritária
- **Toque seguro** com targets ≥40px para operação com dedos
- **Estética profissional** flat, escura, com accent cyan elétrico
- **Eficiência gráfica** para hardware embarcado (sem efeitos caros)

### Paleta de Cores (Design Tokens)

| Token | Valor | Uso |
|-------|-------|-----|
| `COLOR_BG_DARK` | `#090B0F` | Background principal |
| `COLOR_BG_SURFACE` | `#121620` | Cards e superfícies |
| `COLOR_BG_ELEVATED` | `#191E2A` | Superfícies elevadas |
| `COLOR_BG_HEADER` | `#1E2330` | Barra de header |
| `COLOR_BG_NAV` | `#0F1218` | Barra de navegação |
| `COLOR_ACCENT_PRIMARY` | `#06B6D4` | Accent primário (cyan) |
| `COLOR_ACCENT_BRIGHT` | `#22D5F0` | Valores importantes |
| `COLOR_ACCENT_CYAN` | `#00E5FF` | LED de efeitos ativos |
| `COLOR_TEXT_PRIMARY` | `#F2F5F7` | Texto principal |
| `COLOR_TEXT_SECONDARY` | `#A5ADBA` | Texto secundário |
| `COLOR_TEXT_MUTED` | `#687181` | Texto desativado/muted |
| `COLOR_METER_SAFE` | `#00E676` | Meters: -60 a -12dB |
| `COLOR_METER_WARNING` | `#FFD740` | Meters: -12 a -3dB |
| `COLOR_METER_CLIP` | `#FF5252` | Meters: -3 a 0dB |
| `COLOR_LINK_OK` | `#66BB6A` | Link conectado |
| `COLOR_LINK_LOST` | `#EF5350` | Link perdido |
| `COLOR_FS_PRESSED` | `#FFA726` | Footswitch pressionado |

### Tipografia

| Tamanho | Uso | Exemplo |
|---------|-----|---------|
| 24-32px | Hero (nota musical) | `A3` |
| 16-20px | Valores importantes | `-8.2 dB` |
| 13-16px | Labels primários | `HARMONY`, `REVERB` |
| 11-13px | Labels secundários | `FS1`, `LINK` |
| 10px | Diagnósticos (raro) | versões de firmware |

### Componentes Principais

#### 1. Navigation Bar (36px height)
- 4 tabs: **PERF | FX | PRESET | SET**
- Tab ativa: border top 3px cyan + texto cyan
- Target: 80×36px cada (ideal para toque)

#### 2. Header (28px height)
- Nome do preset atual
- Indicador de link (● LINK OK / ● LINK LOST)
- Sem títulos genéricos ("VOXP4 PERFORMANCE")

#### 3. VuMeter Horizontal (52px height × 2 rows)
- Formato horizontal otimizado para 320px width
- Zones coloridas: green → yellow → red
- Peak hold marker vermelho
- Valor numérico em dB à direita
- Fast attack, controlled release

#### 4. Pitch Display (40px height)
- Nota musical em destaque (ex: `A3`)
- Frequência abaixo (ex: `220.1 Hz`)
- Status VOICED/UNVOICED
- Diferença em cents (opcional)

#### 5. Effect Cards (56px height)
- 4 cards: HARMONY, REVERB, DELAY, LIMITER
- Estado ON: border cyan + LED cyan brilhante
- Estado OFF: border sutil + LED cinza
- NÃO transforma card inteiro em verde
- Mostra 1 parâmetro relevante (ex: `+3rd`, `18%`)

#### 6. Footswitch Bar (36px height)
- FS1 e FS2 lado a lado
- Ação resumida abaixo do número
- Highlight amber quando pressionado

### Mockups Disponíveis

| Arquivo | Descrição | Dimensões |
|---------|-----------|-----------|
| `performance_screen.svg` | Tela principal de performance | 320×240 |
| `fx_chain_screen.svg` | Cadeia de efeitos | 320×240 |
| `effect_edit_screen.svg` | Editor de parâmetros | 320×240 |
| `footswitch_screen.svg` | Configuração de footswitches | 320×240 |
| `presets_screen.svg` | Gerenciamento de presets | 320×240 |
| `system_screen.svg` | Informações do sistema | 320×240 |
| `boot_screen.svg` | Tela de boot/loading | 320×240 |

### Hierarquia Visual da Performance Screen

```
┌────────────────────────────────────┐
│ PRESET NAME              LINK ●    │  ← Header 28px
├────────────────────────────────────┤
│ IN   [████████████░░]    -8.2 dB   │  ← Meters 52px
│ OUT  [██████████████░]   -4.1 dB   │
├────────────────────────────────────┤
│          A3                        │  ← Pitch 40px
│       220.1 Hz      VOICED         │
├────────────────────────────────────┤
│ HARMONY   REVERB   DELAY   LIMIT   │  ← Effects 56px
│   ●ON       ●ON      OFF     ●ON   │
├────────────────────────────────────┤
│ FS1 HARMONY      FS2 REVERB        │  ← Footswitch 36px
├────────────────────────────────────┤
│ PERF │  FX  │ PRESET │  SET        │  ← Nav 36px
└────────────────────────────────────┘
Total: 240px (content 204px + nav 36px)
```

### Fluxo de Navegação

```
PERF (Performance) ─┬─ FX Chain ── Effect Edit
                    ├─ Presets ─── Load/Save/Delete
                    ├─ Footswitch ─ Mode/Action config
                    └─ System ──── VoxLink/Audio/Errors
```

### Critérios de Aceitação Visual

✅ Parece processador vocal musical (não demo LVGL genérica)
✅ Hierarquia clara em <1 segundo de leitura
✅ Informação musical > informação técnica
✅ Efeitos ON visíveis sem grandes áreas verdes
✅ Touch targets ≥40px para operação segura
✅ Nada ultrapassa 320×240 viewport
✅ Legível em condições de palco (alto contraste)
✅ Consistente em todas as telas

### Validação Hardware

- **Testado em**: ESP32 CYD (ST7789, 320×240, XPT2046)
- **LVGL**: v8.4.0
- **Fontes**: Montserrat 10/12/14/16/24 compiladas
- **RAM usage**: ~30% (100KB / 320KB)
- **Flash usage**: ~19% (640KB / 3.3MB)

### Notas de Implementação

- SVGs são representações fiéis do que é renderizável em LVGL no ESP32
- Sem gradientes complexos, sombras ou blur (caros para GPU embarcada)
- Cores sólidas, borders simples, barras e indicadores geométricos
- Atualizações de telemetria não recriam objetos LVGL
- Screen manager com containers persistentes (show/hide via flags)

---

**VoxP4 Control Surface Design System v2**  
*Flat, Dark, Musical, Stage-Ready*
