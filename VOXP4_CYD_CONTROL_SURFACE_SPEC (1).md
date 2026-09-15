# VoxP4 CYD Control Surface — Especificação Técnica

**Status:** especificação de referência para implementação  
**Data:** 2026-09-15  
**Projeto alvo:** VoxP4 + ESP32-P4  
**Controlador:** ESP32 Cheap Yellow Display, variante ESP32-2432S028R com **duas portas USB**  
**Referência de hardware:** [`ovelhaaa/beatCYDa`](https://github.com/ovelhaaa/beatCYDa), branch `main`

---

## 1. Objetivo

Este documento especifica uma control surface dedicada ao VoxP4 usando um ESP32 Cheap Yellow Display (CYD) com touchscreen resistivo e dois footswitches.

O CYD deve executar exclusivamente funções de interface e controle:

- interface gráfica touchscreen;
- leitura dos dois footswitches;
- edição dos parâmetros do VoxP4;
- seleção e ativação/desativação de efeitos;
- configuração da função dos footswitches;
- seleção e gerenciamento de presets;
- visualização de meters, pitch, estado de voicing e telemetria;
- comunicação bidirecional com o ESP32-P4.

O ESP32-P4 continua sendo a autoridade sobre:

- processamento de áudio;
- pitch tracking;
- harmonizer;
- reverb;
- dynamics/limiter;
- cadeia de efeitos;
- presets;
- estado real dos parâmetros;
- telemetria de DSP.

O CYD **não deve participar do caminho de áudio**.

---

# 2. Princípio de arquitetura

```text
┌───────────────────────────────┐
│        CYD / ESP32            │
│                               │
│  LVGL                         │
│  Touch XPT2046                │
│  Footswitch 1                 │
│  Footswitch 2                 │
│  UI state/cache               │
│  VoxP4 control protocol       │
│                               │
└───────────────┬───────────────┘
                │
          UART full duplex
             3.3 V TTL
                │
┌───────────────▼───────────────┐
│          ESP32-P4             │
│                               │
│  Protocol task                │
│  Parameter/state manager      │
│  Preset manager               │
│  Telemetry                    │
│                               │
│  VoxP4 DSP                    │
│   ├─ input                    │
│   ├─ pitch analysis           │
│   ├─ harmonizer               │
│   ├─ dry alignment            │
│   ├─ dynamics / limiter       │
│   ├─ reverb                   │
│   └─ output                   │
└───────────────────────────────┘
```

Regra fundamental:

> O P4 é a **fonte de verdade** de todos os parâmetros do VoxP4.  
> O CYD envia intenções de controle e exibe o estado confirmado pelo P4.

---

# 3. Hardware CYD confirmado pelo BeatCYDa

## 3.1 Variante da placa

O hardware usado pelo BeatCYDa corresponde à variante do ESP32-2432S028R de duas portas USB, configurada para:

- ESP32 clássico;
- display de 2,8";
- resolução física de 240 × 320;
- uso em landscape como 320 × 240;
- controlador de display **ST7789**;
- touch resistivo **XPT2046**;
- LovyanGFX;
- SPI com DMA;
- sem necessidade de PSRAM para esta aplicação.

O arquivo de referência para pinout no projeto é:

```text
src/CYD_Config.h
```

A configuração de baixo nível do display/touch está em:

```text
src/ui/LGFX_CYD.h
```

---

# 4. Pinout confirmado

## 4.1 Display ST7789

Configuração ativa no BeatCYDa:

| Função | GPIO |
|---|---:|
| TFT SCLK | 14 |
| TFT MISO | 12 |
| TFT MOSI | 13 |
| TFT CS | 15 |
| TFT DC | 2 |
| TFT RESET | -1 / reset da placa |
| TFT Backlight | 21 |

Parâmetros:

```text
SPI write: 40 MHz
SPI read:  16 MHz
SPI mode:  0
DMA:       SPI_DMA_CH_AUTO
rotation:  1 na aplicação
```

Resolução lógica esperada:

```text
320 × 240 landscape
```

Resolução física configurada no painel:

```text
240 × 320
```

---

## 4.2 Touch XPT2046

Configuração ativa:

| Função | GPIO |
|---|---:|
| Touch SCLK | 25 |
| Touch MISO | 39 |
| Touch MOSI | 32 |
| Touch CS | 33 |
| Touch IRQ | não utilizado (`-1`) |

O BeatCYDa usa SPI por software para o touch:

```text
spi_host = -1
frequency = 1 MHz
```

Calibração atual:

```text
TouchMinX = 240
TouchMaxX = 3800
TouchMinY = 3700
TouchMaxY = 200
```

Offset de rotação:

```text
2
```

Essa configuração deve ser preservada inicialmente no controlador VoxP4.

---

## 4.3 SD Card

Pinout existente:

| Função | GPIO |
|---|---:|
| SD SCLK | 18 |
| SD MISO | 19 |
| SD MOSI | 23 |
| SD CS | 5 |

### Decisão para o VoxP4 controller

O cartão SD local **não é necessário na primeira versão**.

Presets e estado persistente devem residir preferencialmente no ESP32-P4.

Assim, GPIO 18/19 podem ser reutilizados para uma UART dedicada.

**Não inserir cartão SD quando esses pinos estiverem sendo reutilizados.**

---

# 5. Pinout proposto para o controlador VoxP4

## 5.1 Perfil recomendado: `VOXP4_CONTROLLER_NO_SD`

| Função | GPIO CYD | Direção |
|---|---:|---|
| TFT SCLK | 14 | saída |
| TFT MISO | 12 | entrada |
| TFT MOSI | 13 | saída |
| TFT CS | 15 | saída |
| TFT DC | 2 | saída |
| TFT BL | 21 | saída |
| Touch CLK | 25 | saída |
| Touch MISO | 39 | entrada |
| Touch MOSI | 32 | saída |
| Touch CS | 33 | saída |
| VoxP4 UART TX | **18** | saída |
| VoxP4 UART RX | **19** | entrada |
| Footswitch 1 | **22** | entrada |
| Footswitch 2 | **27** | entrada |

GPIO 18 e 19 deixam de pertencer ao SD neste perfil.

GPIO 22 e GPIO 27 são usados como entradas digitais dos footswitches.

---

## 5.2 UART

Configuração inicial recomendada:

```text
UART:       UART2 / HardwareSerial
Baud:       921600
Data bits:  8
Parity:     none
Stop bits:  1
Flow ctrl:  none
Logic:      3.3 V
Mode:       full duplex
```

Mapeamento:

```cpp
RX = GPIO19
TX = GPIO18
```

Exemplo conceitual:

```cpp
VoxSerial.begin(
    921600,
    SERIAL_8N1,
    19, // RX
    18  // TX
);
```

Fallback de robustez:

```text
460800 baud
```

Se CYD e P4 estiverem dentro do mesmo gabinete e ligados por fios curtos, UART TTL é suficiente.

Para cabo externo longo, especialmente em palco, considerar futuramente:

- RS-485;
- CAN;
- transceptores diferenciais;
- proteção ESD adicional.

---

# 6. Footswitches

## 6.1 Tipo elétrico

Recomendação:

- footswitch momentâneo físico;
- contato normalmente aberto;
- fechamento para GND;
- lógica ativa em LOW.

A distinção entre **momentary** e **latching** deve ser feita em software.

---

## 6.2 Ligação básica

```text
3.3 V
  │
 [10 kΩ]      opcional se INPUT_PULLUP for suficiente
  │
  ├──────── GPIO
  │
 [100 nF]
  │
 GND

GPIO ──[1 kΩ]── Tip do jack
GND ─────────── Sleeve do jack
```

Para protótipo pode-se iniciar somente com:

```text
GPIO -> INPUT_PULLUP
footswitch -> GND
```

Para versão final de pedal recomenda-se:

- resistor série de 470 Ω a 2,2 kΩ;
- capacitor de 10 nF a 100 nF;
- proteção ESD no conector;
- pull-up externo de ~10 kΩ;
- debounce adicional por software.

---

## 6.3 GPIOs

```text
FS1 = GPIO22
FS2 = GPIO27
```

Ambos devem ser configurados como:

```cpp
INPUT_PULLUP
```

Estado:

```text
HIGH = solto
LOW  = pressionado
```

---

## 6.4 Debounce

Parâmetros iniciais:

```text
poll interval:       2–5 ms
debounce stable:     15 ms
long press:          500 ms
double press window: 300 ms
```

Não disparar uma ação diretamente na primeira mudança elétrica.

Gerar primeiro eventos semânticos:

```text
PRESS
RELEASE
LONG_PRESS
DOUBLE_PRESS
```

---

# 7. Modos dos footswitches

Cada footswitch possui configuração independente.

## 7.1 Momentary

Exemplo:

```text
PRESS   -> Harmony ON
RELEASE -> Harmony OFF
```

ou:

```text
PRESS   -> Reverb Freeze ON
RELEASE -> Reverb Freeze OFF
```

---

## 7.2 Latching

O footswitch físico continua momentâneo.

O firmware implementa toggle:

```text
PRESS #1 -> effect ON
PRESS #2 -> effect OFF
PRESS #3 -> effect ON
...
```

---

## 7.3 Ações configuráveis

Cada footswitch deve suportar pelo menos:

```text
Effect Toggle
Effect Momentary
Global Bypass
Harmony Toggle
Harmony Momentary
Reverb Toggle
Reverb Freeze
Delay Toggle
Tap Tempo
Preset Next
Preset Previous
Preset Select
Scene Next
Scene Previous
Mute Harmony
Dry/Wet Morph
Custom Action ID
```

Estrutura sugerida:

```cpp
struct FootswitchConfig {
    uint8_t mode;
    uint16_t pressAction;
    uint16_t releaseAction;
    uint16_t longPressAction;
    uint16_t doublePressAction;
};
```

---

# 8. Stack de software do CYD

## 8.1 Base

A configuração atual do BeatCYDa usa:

```text
PlatformIO
platform = espressif32@6.5.0
board = esp32dev
framework = arduino
flash mode = dio
flash clock = 40 MHz
upload speed = 460800
monitor = 115200
```

Dependências atuais relevantes:

```text
LovyanGFX 1.1.12
ArduinoJson 6.21.5
```

Para o controlador VoxP4 adicionar:

```text
LVGL
```

A versão de LVGL deve ser fixada explicitamente no `platformio.ini`.

Não usar dependência sem versão fixa em firmware de produção.

---

# 9. Estratégia LVGL

O BeatCYDa atual renderiza diretamente com LovyanGFX.

Para o VoxP4 controller, a arquitetura recomendada é:

```text
LVGL
  ↓
display flush callback
  ↓
LovyanGFX
  ↓
ST7789
```

Touch:

```text
XPT2046 / LovyanGFX
  ↓
LVGL indev read callback
```

Isso permite reaproveitar a configuração de hardware já validada no BeatCYDa e usar LVGL somente como camada de interface.

---

# 10. Buffer de display

Não usar framebuffer full-screen duplo.

RGB565:

```text
320 × 240 × 2 bytes = 153600 bytes
```

Dois framebuffers consumiriam:

```text
307200 bytes
```

Isso é desnecessário no ESP32 clássico.

## Configuração recomendada

Buffer parcial de 20 linhas:

```text
320 × 20 × 2 = 12800 bytes
```

Double buffer:

```text
25600 bytes
```

Alternativa de 24 linhas:

```text
320 × 24 × 2 = 15360 bytes/buffer
30720 bytes total
```

Configuração inicial recomendada:

```text
RGB565
2 draw buffers
320 × 20 pixels cada
```

---

# 11. Taxas de atualização

Metas:

| Subsistema | Frequência |
|---|---:|
| LVGL task handler | 5–10 ms |
| UI visual | até ~30 FPS |
| Touch | ~60 Hz |
| Footswitch scan | 200–500 Hz |
| Meters recebidos | 20–30 Hz |
| Pitch display | 10–20 Hz |
| CPU/diagnostics | 2–5 Hz |
| Heartbeat | 2 Hz |

O `BeatCYDa` já usa referências próximas de:

```text
UiFrameMs = 33 ms
UiTaskTickMs = 16 ms
TouchPollMs = 16 ms
```

Esses valores são adequados como ponto de partida.

---

# 12. Regras de performance gráfica

Evitar:

- transparência excessiva;
- sombras grandes;
- blur;
- gradientes complexos em elementos animados;
- animações full-screen;
- redraw completo sem necessidade;
- waveform/spectrogram em alta taxa.

Preferir:

- regiões parcialmente invalidadas;
- sliders;
- barras;
- botões grandes;
- labels;
- VU meters simples;
- cores sólidas;
- animações curtas;
- peak indicators discretos.

Meta subjetiva:

> Interface deve responder imediatamente ao toque mesmo quando os meters estão ativos.

---

# 13. Arquitetura de tasks no CYD

Configuração proposta:

```text
CORE 0
├─ VoxLink RX task
├─ VoxLink TX queue
├─ protocol parser
├─ footswitch task
└─ state synchronization

CORE 1
└─ LVGL/UI task
```

O ESP32 deve permanecer fixo em:

```text
240 MHz
```

Sem light sleep durante uso normal da control surface.

---

## 13.1 LVGL task

Sugestão:

```text
core:      1
priority:  3
stack:     10–12 KB
period:    5–10 ms
```

Somente essa task deve chamar APIs LVGL.

Callbacks vindos de outras tasks não devem alterar widgets diretamente.

Usar queue/event para transportar mudanças de estado à UI.

---

## 13.2 UART RX task

```text
core:      0
priority:  6
stack:     4 KB
blocking:  sim
```

Responsabilidades:

- receber bytes;
- reconstruir frames;
- validar CRC;
- atualizar estatísticas;
- encaminhar mensagens válidas ao state manager.

---

## 13.3 TX

Usar fila.

Nenhum callback de UI deve bloquear aguardando UART.

Fluxo:

```text
UI event
   ↓
ControlEvent
   ↓
TX queue
   ↓
UART writer
```

---

## 13.4 Footswitch task

```text
core:      0
priority:  7
period:    2–5 ms
```

Responsabilidades:

- leitura GPIO;
- debounce;
- identificação PRESS/RELEASE;
- long press;
- double press;
- transformação em `ACTION`;
- envio imediato ao P4.

Footswitch não deve depender da taxa de atualização da UI.

---

# 14. Protocolo CYD ↔ VoxP4

Nome sugerido:

```text
VoxLink
```

Versão inicial:

```text
VoxLink Protocol v1
```

---

# 15. Frame binário

Formato recomendado:

```text
+--------+--------+
| 0xA5   | 0x5A   |  SOF
+--------+--------+
| VER             |  uint8
+-----------------+
| TYPE            |  uint8
+-----------------+
| FLAGS           |  uint8
+-----------------+
| SEQ             |  uint16 LE
+-----------------+
| LENGTH          |  uint16 LE
+-----------------+
| PAYLOAD ...     |
+-----------------+
| CRC16           |  uint16 LE
+-----------------+
```

---

## 15.1 Requisitos

- framing independente de timing;
- CRC16;
- sequence number;
- tamanho explícito;
- versionamento;
- ACK somente quando necessário;
- recuperação após byte perdido;
- parser sem alocação dinâmica por frame.

Máximo inicial sugerido:

```text
payload <= 512 bytes
```

Snapshots grandes podem ser fragmentados.

---

# 16. Tipos de mensagem mínimos

```text
0x01 HELLO
0x02 HELLO_ACK
0x03 CAPS_REQUEST
0x04 CAPS_RESPONSE

0x10 GET_STATE
0x11 STATE_SNAPSHOT
0x12 SET_PARAM
0x13 PARAM_CHANGED

0x20 ACTION
0x21 ACTION_RESULT

0x30 PRESET_LIST_REQUEST
0x31 PRESET_LIST
0x32 PRESET_LOAD
0x33 PRESET_SAVE
0x34 PRESET_CHANGED

0x40 METER_FRAME
0x41 PITCH_FRAME
0x42 DSP_STATUS

0x50 FOOTSWITCH_CONFIG
0x51 FOOTSWITCH_STATE

0x70 HEARTBEAT
0x71 ACK
0x72 NACK
0x73 ERROR
```

---

# 17. Identificação de parâmetros

Não enviar strings continuamente.

Usar IDs numéricos estáveis.

Exemplo:

```text
0x0100 Harmony Enabled
0x0101 Harmony Interval
0x0102 Harmony Gain
0x0103 Harmony Pan
0x0104 Harmony Attack
0x0105 Harmony Release

0x0200 Reverb Enabled
0x0201 Reverb Mix
0x0202 Reverb Decay
0x0203 Reverb Predelay
0x0204 Reverb Damping

0x0300 Limiter Enabled
0x0301 Limiter Threshold
0x0302 Limiter Release
```

A tabela oficial de IDs deve existir em um header compartilhável.

Exemplo:

```text
protocol/voxp4_param_ids.h
```

Idealmente gerar automaticamente bindings para ambos os firmwares a partir de uma descrição única.

---

# 18. `SET_PARAM`

Payload conceitual:

```cpp
struct SetParam {
    uint16_t paramId;
    uint8_t valueType;
    Value value;
};
```

Tipos:

```text
BOOL
INT32
UINT32
FLOAT32
ENUM
```

Evitar `double`.

---

# 19. Coalescing de controles

Um slider pode gerar dezenas de eventos por segundo.

Não é necessário transmitir cada pixel de movimento.

Regra:

```text
enviar imediatamente o primeiro valor;
durante drag limitar a ~50–100 updates/s;
sempre enviar o valor final no release.
```

Isso mantém UI responsiva sem criar flood de mensagens.

---

# 20. State authority

Fluxo correto:

```text
CYD:
user move slider
    ↓
SET_PARAM id=X value=0.72
    ↓

P4:
valida
aplica/suaviza
atualiza estado
    ↓
PARAM_CHANGED id=X value=0.72
    ↓

CYD:
atualiza estado confirmado
```

O CYD pode fazer atualização visual otimista, mas deve reconciliar com o valor retornado pelo P4.

---

# 21. Sincronização no boot

Sequência:

```text
CYD boot
  ↓
HELLO
  ↓
P4 HELLO_ACK
  ↓
CAPS_REQUEST
  ↓
CAPS_RESPONSE
  ↓
GET_STATE
  ↓
STATE_SNAPSHOT
  ↓
UI READY
```

Antes de receber o snapshot:

- controles podem aparecer desabilitados;
- mostrar `CONNECTING`;
- não assumir valores default do DSP.

---

# 22. Reconnect

Se link cair:

```text
LINK LOST
```

O CYD:

1. preserva somente estado visual temporário;
2. desabilita comandos destrutivos;
3. continua tentando `HELLO`;
4. ao reconectar pede snapshot completo;
5. descarta estado local antigo;
6. volta a seguir o P4.

Timeout recomendado:

```text
heartbeat every 500 ms
link lost after 1500–2000 ms
```

---

# 23. Telemetria

O P4 deve fornecer telemetria desacoplada do áudio.

## `METER_FRAME`

Inicialmente:

```text
input_peak_db
input_rms_db
harmony_peak_db
harmony_rms_db
output_peak_db
output_rms_db
limiter_gain_reduction_db
```

Formato:

```text
float32
```

Taxa:

```text
20–30 Hz
```

---

## `PITCH_FRAME`

```text
frequency_hz
midi_note_float
confidence
voiced
detected_note
```

Taxa:

```text
10–20 Hz
```

---

## `DSP_STATUS`

```text
cpu_load
audio_buffer_load
underrun_count
overrun_count
sample_rate
block_size
temperature optional
free_heap optional
```

Taxa:

```text
2–5 Hz
```

---

# 24. UI — princípios gerais

Resolução:

```text
320 × 240
```

Uso principal:

- em pé;
- palco;
- leitura rápida;
- touchscreen resistivo;
- interação com dedo.

Portanto:

- touch targets grandes;
- contraste alto;
- poucas informações por tela;
- parâmetros críticos acessíveis em 1–2 toques;
- ações de performance não escondidas em menus profundos.

Tamanho mínimo recomendado de alvo:

```text
40 × 40 px
```

Preferível:

```text
44–48 px
```

---

# 25. Estrutura global da UI

Telas principais:

```text
1. PERFORMANCE
2. FX CHAIN
3. EFFECT EDIT
4. FOOTSWITCH
5. PRESETS
6. SYSTEM
```

Não é obrigatório mostrar todas na barra principal.

A navegação pode ter:

```text
PERF | FX | PRESET | SET
```

e abrir `EFFECT EDIT` ao tocar em um efeito.

---

# 26. Tela PERFORMANCE

Tela padrão ao ligar.

Objetivo:

> Mostrar imediatamente se o sistema está funcionando e permitir operação ao vivo sem entrar em menus.

Layout conceitual:

```text
┌──────────────────────────────────────┐
│ VOXP4  P03 Lead Air       ● LINK    │
├──────────────────────────────────────┤
│ IN   ███████████░░        -8.2 dB   │
│ OUT  █████████████░       -5.1 dB   │
│                                      │
│ Pitch: A3  220.1Hz        VOICED     │
│                                      │
│ [HARMONY] [REVERB] [LIMIT] [DELAY]  │
│    ON        ON       ON      OFF    │
│                                      │
├──────────────────────────────────────┤
│ FS1 HARMONY          FS2 REVERB      │
└──────────────────────────────────────┘
```

Elementos:

- nome do preset;
- estado do link;
- input meter;
- output meter;
- pitch;
- voiced/unvoiced;
- estado de 3–4 efeitos principais;
- labels dos footswitches;
- indicação visual quando FS está pressionado.

---

# 27. Meter behavior

Recomendação:

```text
attack visual:    rápido
release visual:   200–400 ms
peak hold:        500–1000 ms
clip indicator:   latched por ~1 s
```

Não redesenhar a tela inteira para atualizar meters.

Invalidar somente os widgets envolvidos.

---

# 28. Tela FX CHAIN

Objetivo:

- visualizar cadeia;
- ligar/desligar efeitos;
- abrir editor de cada bloco.

Exemplo:

```text
INPUT
  ↓
[Pitch]
  ↓
[Harmony]
  ↓
[Dynamics]
  ↓
[Reverb]
  ↓
[Limiter]
  ↓
OUTPUT
```

Cada card exibe:

```text
nome
ON/OFF
1–2 parâmetros resumo
```

Exemplo:

```text
HARMONY
+3rd / -9.0 dB
ON
```

---

# 29. Editor de efeito

Evitar muitos knobs pequenos.

Preferir:

- slider horizontal;
- valor grande;
- botões `-` e `+`;
- toggle;
- enum segmented buttons.

Layout:

```text
┌──────────────────────────────────────┐
│ < HARMONY                       ON   │
├──────────────────────────────────────┤
│ INTERVAL                  +3rd       │
│ [-]  [──────●────────]          [+] │
│                                      │
│ LEVEL                   -9.0 dB      │
│ [-]  [────────●──────]          [+] │
│                                      │
│ PAN                        CENTER    │
│      [L] [CENTER] [R]               │
└──────────────────────────────────────┘
```

Parâmetros adicionais podem usar páginas:

```text
1/2
2/2
```

---

# 30. Tela FOOTSWITCH

Exemplo:

```text
┌──────────────────────────────────────┐
│ FOOTSWITCHES                         │
├──────────────────────────────────────┤
│ FS1                                  │
│ Mode: [LATCH]  MOMENTARY             │
│ Action: Harmony Toggle               │
│ Hold:   Global Bypass                │
│                                      │
│ FS2                                  │
│ Mode: LATCH  [MOMENTARY]             │
│ Action: Reverb Freeze                │
│ Hold:   Preset Next                  │
└──────────────────────────────────────┘
```

A UI deve permitir:

- modo;
- ação press;
- ação release;
- long press;
- double press;
- opcionalmente LED/feedback behavior.

---

# 31. Tela PRESETS

O armazenamento principal é no P4.

CYD requisita lista:

```text
PRESET_LIST_REQUEST
```

P4 devolve:

```text
id
name
flags
```

UI:

```text
01 Clean
02 Wide Harmony
03 Plate Vocal
04 Octave Air
05 Big Chorus
...
```

Ações:

```text
LOAD
SAVE
SAVE AS
RENAME
```

Operações destrutivas exigem confirmação.

---

# 32. Tela SYSTEM

Mostrar:

```text
VoxP4 firmware
CYD firmware
VoxLink version
sample rate
block size
DSP CPU
P4 free heap
CYD free heap
UART RX errors
CRC errors
reconnect count
audio underruns
uptime
```

Também:

```text
Touch calibration
Display brightness
Telemetry rate
UART baud
Factory defaults
```

---

# 33. Feedback visual dos footswitches

Footswitch precisa ter feedback imediato.

Ao pressionar:

```text
FS1 card -> estado PRESSED
```

Quando ação é confirmada pelo P4:

```text
efeito -> estado real confirmado
```

Nunca usar somente animação local como prova de que o efeito foi ativado.

---

# 34. Preservação de áudio

Nenhuma task do CYD deve poder bloquear o P4.

No P4:

```text
UART ISR / driver
   ↓
protocol RX queue
   ↓
control task
   ↓
parameter manager
   ↓
atomic/smoothed DSP parameters
```

Nunca:

```text
UART callback -> executar DSP pesado
```

Nunca:

```text
UART callback -> adquirir mutex usado pelo audio callback por tempo indefinido
```

---

# 35. Parameter smoothing no P4

Mudanças de slider podem produzir zipper noise se aplicadas diretamente.

O protocolo envia o destino.

O DSP realiza smoothing.

Exemplo:

```text
CYD:
mix = 0.67

P4:
target_mix = 0.67

audio:
current_mix += alpha * (target_mix - current_mix)
```

O CYD não deve tentar fazer smoothing de áudio.

---

# 36. Limites de tráfego

A 921600 baud há ampla margem para:

- comandos;
- snapshots;
- meters;
- pitch;
- status.

Ainda assim, impor limites:

```text
meters:          <= 30 Hz
pitch:           <= 20 Hz
status:          <= 5 Hz
parameter drag:  <= 100 Hz por controle
```

Não enviar:

- áudio PCM;
- waveform contínua;
- FFT completa;
- spectrogram em alta taxa.

---

# 37. Segurança do parser

O parser deve rejeitar:

- versão incompatível;
- payload maior que máximo;
- CRC inválido;
- type inválido;
- parâmetro inexistente;
- enum fora da faixa.

Nunca confiar no comprimento recebido sem validar.

---

# 38. Compatibilidade de protocolo

`HELLO` deve carregar:

```text
protocol_major
protocol_minor
firmware_major
firmware_minor
device_type
capability_flags
```

Regra:

```text
major diferente -> incompatível
minor diferente -> negociar capabilities
```

---

# 39. Capabilities

O P4 deve informar ao CYD quais módulos existem.

Exemplo:

```text
CAP_HARMONY
CAP_REVERB
CAP_LIMITER
CAP_DELAY
CAP_COMPRESSOR
CAP_PITCH_TELEMETRY
CAP_PRESETS
CAP_SCENES
```

Assim a UI não precisa assumir que todo build do VoxP4 possui todos os efeitos.

---

# 40. Modelo de dados local

Sugestão:

```cpp
struct VoxUiState {
    bool linkUp;
    uint16_t presetId;
    char presetName[32];

    HarmonyState harmony;
    ReverbState reverb;
    DynamicsState dynamics;
    LimiterState limiter;

    MeterState meters;
    PitchState pitch;
    DspStatus dsp;

    FootswitchConfig fs1;
    FootswitchConfig fs2;
};
```

UI lê exclusivamente esse state/cache.

Protocol task atualiza o cache através de mensagens/eventos thread-safe.

---

# 41. Memória

Objetivo:

- nenhuma alocação dinâmica em loops críticos;
- buffers UART fixos;
- buffers LVGL fixos;
- limite explícito de strings;
- sem carregar assets grandes em RAM.

Imagens devem ser pequenas e, preferencialmente:

- ícones simples;
- vetores desenhados;
- símbolos/fontes.

Evitar bitmaps full-screen.

---

# 42. Boot do CYD

Fluxo sugerido:

```text
1. boot
2. CPU 240 MHz
3. init serial debug
4. init GPIO footswitch
5. init ST7789
6. init touch
7. init LVGL
8. start VoxLink
9. show CONNECTING
10. HELLO
11. sync state
12. show PERFORMANCE
```

Tempo de boot deve ser minimizado.

---

# 43. Debug serial

Evitar usar UART0 para comunicação normal com o P4.

Manter:

```text
Serial / UART0 = debug e upload
UART dedicada  = VoxLink
```

Isso permite:

- monitor serial durante desenvolvimento;
- logs;
- programação sem desconectar o P4;
- diagnóstico de link separado.

---

# 44. Logging

Níveis:

```text
ERROR
WARN
INFO
DEBUG
TRACE
```

Produção:

```text
INFO ou WARN
```

Não imprimir um log por meter frame.

Contadores de erro devem aparecer na tela SYSTEM.

---

# 45. Brightness

Backlight está em:

```text
GPIO21
```

Manter controle de brilho configurável.

Persistir:

```text
brightness
```

em NVS do CYD.

Apenas configurações específicas da control surface devem ficar localmente.

---

# 46. Persistência local no CYD

NVS pode guardar:

```text
brightness
touch calibration
preferred telemetry rate
last UI page
footswitch UI preferences se desejado
```

Não duplicar presets completos do VoxP4 por padrão.

P4 continua sendo autoridade de preset.

---

# 47. Falha do P4

Se o link cair durante apresentação:

- tela deve indicar claramente `LINK LOST`;
- não travar;
- footswitches deixam de assumir mudança de efeito;
- continuar reconexão automática;
- manter display e touch funcionais;
- mostrar última telemetria como stale, não como atual.

Sugestão:

```text
meters cinza
LINK LOST em vermelho
```

---

# 48. Falha do CYD

O P4 deve continuar processando áudio normalmente mesmo se:

- CYD reiniciar;
- UART desconectar;
- touchscreen travar;
- cabo for removido.

A existência da control surface nunca deve ser requisito para o DSP funcionar.

---

# 49. Requisitos de UX para palco

A interface deve funcionar sob pressão.

Prioridades:

1. estado ON/OFF de cada efeito;
2. preset atual;
3. nível de entrada/saída;
4. feedback dos footswitches;
5. link status;
6. pitch/voicing;
7. edição detalhada.

Nunca esconder o preset atual em uma tela secundária.

---

# 50. Cores e estados

Sem impor uma paleta específica, os estados devem ser semanticamente consistentes.

Exemplo:

```text
ON        -> destaque forte
OFF       -> neutro
PRESSED   -> highlight momentâneo
WARNING   -> amarelo/laranja
ERROR     -> vermelho
LINK OK   -> verde
LINK LOST -> vermelho
```

Não depender somente de cor.

Combinar cor + texto/ícone.

---

# 51. Touch resistivo — UX

Evitar gestos complexos.

Suportar principalmente:

```text
tap
drag horizontal
press/hold
```

Evitar depender de:

```text
pinch
multi-touch
swipe preciso
```

XPT2046 é single-touch resistivo.

---

# 52. Calibração de touch

Usar os valores do BeatCYDa inicialmente:

```text
X: 240 → 3800
Y: 3700 → 200
```

Adicionar futuramente uma rotina guiada:

```text
touch four corners
compute calibration
save to NVS
```

Ela deve substituir constantes somente após calibração válida.

---

# 53. Testes de hardware

Antes de integrar ao P4:

## Teste 1 — display

- tela inteira;
- orientation;
- cores;
- backlight;
- 30 FPS parcial.

## Teste 2 — touch

- quatro cantos;
- centro;
- drag;
- pressão prolongada;
- sem jitter excessivo.

## Teste 3 — footswitch

- 1000 acionamentos;
- sem double-trigger;
- press/release corretos;
- long press;
- double press.

## Teste 4 — UART loopback

- 921600;
- pacote contínuo;
- CRC;
- zero perda durante 30 minutos.

---

# 54. Testes de integração com VoxP4

## Stress A

```text
meters 30 Hz
pitch 20 Hz
DSP status 5 Hz
slider movimentado continuamente
footswitch acionado repetidamente
```

Critério:

- UI não congela;
- nenhuma perda audível no P4;
- sem underrun adicional.

## Stress B

Desconectar UART e reconectar.

Critério:

- reconexão automática;
- snapshot correto;
- UI converge ao estado real.

## Stress C

Reiniciar apenas o CYD.

Critério:

- áudio do P4 continua;
- CYD reconecta;
- recebe estado atual.

## Stress D

Reiniciar apenas o P4.

Critério:

- CYD detecta link perdido;
- aguarda;
- faz nova negociação;
- recupera estado.

---

# 55. Critérios de aceitação

A primeira versão é considerada funcional quando:

- [ ] ST7789 funciona em 320 × 240 landscape;
- [ ] touch funciona com calibração correta;
- [ ] LVGL roda estável;
- [ ] UI mantém aproximadamente 30 FPS em interação normal;
- [ ] FS1 funciona sem bounce perceptível;
- [ ] FS2 funciona sem bounce perceptível;
- [ ] UART estável em 921600 ou 460800;
- [ ] HELLO/reconnect funcionam;
- [ ] estado é sincronizado pelo P4;
- [ ] Harmony pode ser ligado/desligado pela tela;
- [ ] Reverb pode ser ligado/desligado pela tela;
- [ ] FS1/FS2 podem ser configurados;
- [ ] meters atualizam sem bloquear a UI;
- [ ] pitch/voiced são exibidos;
- [ ] preset atual aparece na tela principal;
- [ ] P4 continua processando áudio se CYD desconectar;
- [ ] nenhum comando de UI roda dentro da audio callback;
- [ ] nenhuma mudança de slider causa zipper noise perceptível.

---

# 56. Estrutura sugerida do firmware CYD

```text
src/
├─ main.cpp
├─ board/
│  ├─ CYD_Config.h
│  ├─ LGFX_CYD.h
│  └─ FootswitchPins.h
├─ ui/
│  ├─ LvglDisplay.cpp
│  ├─ LvglTouch.cpp
│  ├─ UiApp.cpp
│  ├─ UiTheme.cpp
│  ├─ screens/
│  │  ├─ PerformanceScreen.cpp
│  │  ├─ FxChainScreen.cpp
│  │  ├─ EffectEditScreen.cpp
│  │  ├─ FootswitchScreen.cpp
│  │  ├─ PresetScreen.cpp
│  │  └─ SystemScreen.cpp
│  └─ widgets/
│     ├─ VuMeter.cpp
│     ├─ EffectCard.cpp
│     ├─ ParamSlider.cpp
│     └─ FootswitchCard.cpp
├─ protocol/
│  ├─ VoxLink.cpp
│  ├─ VoxLinkParser.cpp
│  ├─ VoxLinkMessages.h
│  ├─ VoxLinkCrc.cpp
│  └─ VoxP4ParamIds.h
├─ control/
│  ├─ ControlManager.cpp
│  ├─ FootswitchManager.cpp
│  └─ StateStore.cpp
├─ transport/
│  ├─ VoxUart.cpp
│  └─ TxQueue.cpp
└─ storage/
   └─ LocalSettings.cpp
```

---

# 57. Estrutura sugerida no ESP32-P4

```text
components/
└─ voxp4_control/
   ├─ voxp4_protocol.cpp
   ├─ voxp4_protocol.h
   ├─ voxp4_param_ids.h
   ├─ voxp4_parameter_manager.cpp
   ├─ voxp4_parameter_manager.h
   ├─ voxp4_telemetry.cpp
   ├─ voxp4_preset_service.cpp
   └─ voxp4_control_task.cpp
```

Comunicação com DSP:

```text
protocol
   ↓
parameter manager
   ↓
atomic target values / lock-free queue
   ↓
audio DSP
```

---

# 58. Migração a partir do BeatCYDa

Reutilizar:

- `CYD_Config.h`;
- pinout ST7789;
- calibração XPT2046;
- configuração LovyanGFX;
- configuração SPI;
- DMA;
- CPU em 240 MHz;
- conhecimento já adquirido sobre a variante de duas USB.

Substituir:

- groovebox UI;
- engine de áudio local;
- PatternStorage;
- WavSampleBank;
- sequencer;
- mixer interno.

Adicionar:

- LVGL;
- UART VoxLink;
- state synchronization;
- two-footcontroller manager;
- telas específicas do VoxP4.

---

# 59. Configuração-base consolidada

```cpp
namespace VoxCydConfig {

constexpr uint16_t ScreenWidth  = 320;
constexpr uint16_t ScreenHeight = 240;
constexpr uint8_t ScreenRotation = 1;

// ST7789
constexpr int TftSclk = 14;
constexpr int TftMiso = 12;
constexpr int TftMosi = 13;
constexpr int TftCs   = 15;
constexpr int TftDc   = 2;
constexpr int TftRst  = -1;
constexpr int TftBacklight = 21;

// XPT2046
constexpr int TouchSclk = 25;
constexpr int TouchMiso = 39;
constexpr int TouchMosi = 32;
constexpr int TouchCs   = 33;
constexpr int TouchIrq  = -1;

constexpr uint16_t TouchMinX = 240;
constexpr uint16_t TouchMaxX = 3800;
constexpr uint16_t TouchMinY = 3700;
constexpr uint16_t TouchMaxY = 200;

// VoxP4 UART — reuses SD bus pins
constexpr int VoxUartTx = 18;
constexpr int VoxUartRx = 19;
constexpr uint32_t VoxUartBaud = 921600;

// Footswitches
constexpr int Footswitch1 = 22;
constexpr int Footswitch2 = 27;

// UI
constexpr uint16_t UiFrameMs = 33;
constexpr uint16_t TouchPollMs = 16;

// Footswitch
constexpr uint16_t FootswitchPollMs = 4;
constexpr uint16_t FootswitchDebounceMs = 15;
constexpr uint16_t FootswitchLongPressMs = 500;
constexpr uint16_t FootswitchDoublePressMs = 300;

}
```

---

# 60. Decisões fechadas para a v1

Para evitar crescimento de escopo, considerar fechadas inicialmente:

```text
CYD = control surface somente
P4 = autoridade de estado e DSP
ST7789 = driver de display
XPT2046 = touch
320×240 landscape
LVGL + LovyanGFX
UART full duplex
921600 baud, fallback 460800
CRC16
protocolo binário
sem JSON durante operação normal
sem SD local
GPIO18/19 = VoxLink
GPIO22/27 = footswitches
2 footswitches momentâneos físicos
momentary/latching em software
telemetria <= 30 Hz
P4 continua funcionando sem CYD
```

---

# 61. Itens explicitamente fora da v1

Não implementar inicialmente:

- Wi-Fi;
- BLE;
- ESP-NOW;
- áudio no CYD;
- streaming de áudio;
- waveform contínua;
- spectrogram;
- edição gráfica complexa de EQ;
- SD local;
- atualização de firmware do P4 através do CYD;
- editor desktop;
- multi-touch.

Esses itens podem ser adicionados depois sem alterar a arquitetura básica.

---

# 62. Fonte de verdade e referências do BeatCYDa

Arquivos usados para consolidar a configuração de hardware:

```text
https://github.com/ovelhaaa/beatCYDa/blob/main/src/CYD_Config.h
https://github.com/ovelhaaa/beatCYDa/blob/main/src/ui/LGFX_CYD.h
https://github.com/ovelhaaa/beatCYDa/blob/main/src/main.cpp
https://github.com/ovelhaaa/beatCYDa/blob/main/platformio.ini
https://github.com/ovelhaaa/beatCYDa/blob/main/README.md
https://github.com/ovelhaaa/beatCYDa/blob/main/touch_e_sd/exemplo_config.txt
```

A variante de duas USB é tratada no material de referência do repositório como a versão com painel ST7789.

---

# 63. Observação sobre pinout proposto

Os pinos de TFT e touch desta especificação vêm diretamente do firmware BeatCYDa.

Os pinos:

```text
GPIO18 / GPIO19 -> VoxLink
GPIO22 / GPIO27 -> footswitches
```

são uma **nova alocação para o projeto VoxP4**.

Ela pressupõe:

- cartão SD não utilizado;
- ausência de conflitos adicionais introduzidos pelo hardware final;
- validação elétrica no protótipo antes de fabricar uma placa filha.

Caso seja necessário manter o SD, o pinout do VoxLink deverá ser revisto.

---

# 64. Próxima etapa recomendada

A implementação deve começar por um firmware mínimo de bring-up:

```text
ST7789
+ XPT2046
+ LVGL
+ FS1
+ FS2
+ UART loopback
```

Somente depois integrar:

```text
VoxLink
→ state sync
→ effects
→ meters
→ presets
```

Isso permite separar problemas de hardware, UI e protocolo antes de conectar o sistema ao caminho de áudio do VoxP4.
