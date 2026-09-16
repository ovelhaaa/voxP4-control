# VoxP4-control

Interface física de controle para o **VoxP4**, baseada em um **ESP32 Cheap Yellow Display (CYD) 2-USB**, com touchscreen resistivo e dois footswitches externos.

O objetivo é transformar o CYD em uma control surface dedicada para o VoxP4, deixando o **ESP32-P4 inteiramente responsável pelo processamento de áudio**.

```text
┌─────────────────────────────┐
│       VoxP4-control         │
│                             │
│  ESP32 CYD 320×240          │
│  ST7789 + XPT2046           │
│                             │
│  LVGL UI                    │
│  Touch                      │
│  Footswitch 1               │
│  Footswitch 2               │
│                             │
└──────────────┬──────────────┘
               │
          VoxLink / UART
               │
┌──────────────▼──────────────┐
│          VoxP4              │
│         ESP32-P4            │
│                             │
│  Pitch tracking             │
│  Harmonizer                 │
│  Dynamics                   │
│  Reverb                     │
│  Limiter                    │
│  Presets                    │
│  DSP / Audio I/O            │
└─────────────────────────────┘
```

---

## Objetivo

O `VoxP4-control` será responsável por:

* interface gráfica touchscreen;
* controle dos parâmetros do VoxP4;
* ativação e bypass de efeitos;
* seleção de presets;
* monitoramento de níveis;
* visualização de pitch e voicing;
* configuração dos footswitches;
* feedback de estado do DSP;
* comunicação bidirecional com o ESP32-P4.

O CYD **não executa DSP de áudio**.

O ESP32-P4 continua sendo a fonte de verdade para:

* parâmetros;
* estado dos efeitos;
* presets;
* cadeia de processamento;
* telemetria;
* processamento de áudio.

---

# Hardware

## Controlador

Hardware alvo:

```text
ESP32-2432S028R
Cheap Yellow Display
2-USB variant
```

Esta revisão utiliza:

```text
Display:     ST7789
Touch:       XPT2046 resistivo
Resolution:  320 × 240 landscape
MCU:         ESP32 clássico
```

A configuração conhecida como funcional vem do projeto:

```text
https://github.com/ovelhaaa/beatCYDa
```

O `beatCYDa` deve ser considerado a referência de hardware deste projeto.

---

# Pinout

## Display ST7789

| Função    | GPIO |
| --------- | ---: |
| TFT SCLK  |   14 |
| TFT MISO  |   12 |
| TFT MOSI  |   13 |
| TFT CS    |   15 |
| TFT DC    |    2 |
| TFT RESET |   -1 |
| Backlight |   21 |

Configuração inicial:

```text
SPI mode:      0
Write clock:   40 MHz
Read clock:    16 MHz
DMA:           enabled
```

---

## Touch XPT2046

| Função     |          GPIO |
| ---------- | ------------: |
| Touch SCLK |            25 |
| Touch MISO |            39 |
| Touch MOSI |            32 |
| Touch CS   |            33 |
| Touch IRQ  | não utilizado |

Calibração de referência:

```text
X min: 240
X max: 3800

Y min: 3700
Y max: 200
```

---

## VoxP4 UART

A comunicação entre o CYD e o ESP32-P4 será feita inicialmente por UART full duplex.

| Função     | GPIO |
| ---------- | ---: |
| TX para P4 |   18 |
| RX do P4   |   19 |

Configuração:

```text
Baud:      921600
Fallback:  460800
Format:    8N1
Voltage:   3.3 V
```

GPIO18 e GPIO19 originalmente pertencem ao barramento do cartão SD.

Por isso, a primeira versão do `VoxP4-control` **não utiliza o cartão SD do CYD**.

---

## Footswitches

| Função       | GPIO |
| ------------ | ---: |
| Footswitch 1 |   22 |
| Footswitch 2 |   27 |

Configuração:

```text
INPUT_PULLUP
active LOW
switch -> GND
```

Os footswitches físicos devem ser preferencialmente momentâneos.

O comportamento pode ser configurado em software como:

```text
Momentary
Latching
```

Além de suportar:

```text
Press
Release
Long Press
Double Press
```

---

# Interface

A interface será implementada usando:

```text
LVGL
+
LovyanGFX
```

Arquitetura gráfica:

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
XPT2046
  ↓
LovyanGFX / input adapter
  ↓
LVGL
```

---

# Telas planejadas

## Performance

Tela principal para uso ao vivo.

Deve mostrar:

* preset atual;
* estado da comunicação;
* input meter;
* output meter;
* pitch;
* voiced/unvoiced;
* efeitos ativos;
* função dos footswitches.

Exemplo conceitual:

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

---

## FX Chain

Visualização da cadeia de efeitos.

Exemplo:

```text
INPUT
  ↓
PITCH
  ↓
HARMONY
  ↓
DYNAMICS
  ↓
REVERB
  ↓
LIMITER
  ↓
OUTPUT
```

Os módulos disponíveis devem ser informados pelo ESP32-P4 através do protocolo de capabilities.

---

## Effect Editor

Edição detalhada de cada efeito.

A interface deve priorizar:

* sliders grandes;
* valores numéricos legíveis;
* toggles;
* botões grandes;
* controles apropriados para touchscreen resistivo.

Evitar interfaces densas com muitos knobs pequenos.

---

## Footswitches

Permite configurar FS1 e FS2.

Exemplos de ações:

```text
Harmony Toggle
Harmony Momentary
Reverb Toggle
Reverb Freeze
Delay Toggle
Global Bypass
Preset Next
Preset Previous
Scene Next
Scene Previous
Tap Tempo
```

---

## Presets

Seleção e gerenciamento dos presets armazenados no ESP32-P4.

O P4 permanece como autoridade do banco de presets.

---

## System

Informações de diagnóstico:

```text
VoxP4 firmware
CYD firmware
VoxLink version
DSP CPU load
sample rate
block size
audio underruns
free heap
UART errors
CRC errors
reconnect count
uptime
```

---

# VoxLink

A comunicação CYD ↔ P4 utiliza o protocolo **VoxLink**.

Primeira versão:

```text
VoxLink v1
```

O protocolo principal será binário.

Estrutura conceitual:

```text
SOF
VERSION
TYPE
FLAGS
SEQ
LENGTH
PAYLOAD
CRC16
```

Exemplo:

```text
0xA5 0x5A
VERSION
TYPE
FLAGS
SEQ
LENGTH
PAYLOAD
CRC16
```

Características:

* framing explícito;
* CRC16;
* sequence number;
* versionamento;
* payload com tamanho explícito;
* recuperação após frames inválidos;
* nenhuma dependência de timing entre pacotes.

---

# Mensagens principais

O protocolo deverá suportar pelo menos:

```text
HELLO
HELLO_ACK

CAPS_REQUEST
CAPS_RESPONSE

GET_STATE
STATE_SNAPSHOT

SET_PARAM
PARAM_CHANGED

ACTION
ACTION_RESULT

PRESET_LIST
PRESET_LOAD
PRESET_SAVE

METER_FRAME
PITCH_FRAME
DSP_STATUS

HEARTBEAT
ACK
NACK
ERROR
```

---

# Fonte de verdade

O ESP32-P4 é sempre a autoridade sobre o estado.

Exemplo:

```text
CYD
 ↓
SET_PARAM reverb_mix = 0.40

P4
 ↓
aplica parâmetro

P4
 ↓
PARAM_CHANGED reverb_mix = 0.40

CYD
 ↓
atualiza estado confirmado
```

Isso permite que parâmetros também sejam modificados por:

* MIDI;
* presets;
* automação;
* outros controladores;
* lógica interna do VoxP4.

---

# Telemetria

O P4 poderá enviar ao CYD:

## Meters

```text
input peak
input RMS
harmony peak
harmony RMS
output peak
output RMS
limiter gain reduction
```

Taxa inicial:

```text
20–30 Hz
```

## Pitch

```text
frequency
MIDI note
confidence
voiced
detected note
```

Taxa inicial:

```text
10–20 Hz
```

## DSP status

```text
CPU load
buffer load
audio underruns
sample rate
block size
free heap
```

Taxa inicial:

```text
2–5 Hz
```

---

# Arquitetura de software

Estrutura inicial sugerida:

```text
src/
├─ main.cpp
│
├─ board/
│  ├─ CYD_Config.h
│  └─ LGFX_CYD.h
│
├─ ui/
│  ├─ LvglDisplay.cpp
│  ├─ LvglTouch.cpp
│  ├─ UiApp.cpp
│  ├─ UiTheme.cpp
│  ├─ screens/
│  └─ widgets/
│
├─ protocol/
│  ├─ VoxLink.cpp
│  ├─ VoxLinkParser.cpp
│  ├─ VoxLinkMessages.h
│  ├─ VoxLinkCrc.cpp
│  └─ VoxP4ParamIds.h
│
├─ control/
│  ├─ ControlManager.cpp
│  ├─ FootswitchManager.cpp
│  └─ StateStore.cpp
│
├─ transport/
│  ├─ VoxUart.cpp
│  └─ TxQueue.cpp
│
└─ storage/
   └─ LocalSettings.cpp
```

---

# Tasks / FreeRTOS

Arquitetura conceitual:

```text
Core 0
├─ UART RX
├─ UART TX
├─ VoxLink parser
├─ State synchronization
└─ Footswitch manager

Core 1
└─ LVGL / UI
```

Somente a task da UI deve manipular objetos LVGL diretamente.

Outras tasks devem enviar eventos através de filas.

---

# UI performance

Metas iniciais:

```text
LVGL handler:       5–10 ms
UI:                 até ~30 FPS
Touch:              ~60 Hz
Footswitch polling: 200–500 Hz
Meters:             20–30 Hz
Pitch:              10–20 Hz
DSP status:         2–5 Hz
```

O display deve usar buffers parciais.

Configuração inicial sugerida:

```text
RGB565
320 × 20 pixels
double buffer
```

Isso consome aproximadamente:

```text
25.6 KB
```

para os buffers gráficos.

---

# Ambiente de build

O projeto deverá inicialmente permanecer próximo ao ambiente já utilizado pelo BeatCYDa.

Exemplo de `platformio.ini`:

```ini
[platformio]
default_envs = esp32-cyd

[env:esp32-cyd]
platform = espressif32@6.5.0
board = esp32dev
framework = arduino

monitor_speed = 115200
upload_speed = 460800

board_build.flash_mode = dio
board_build.f_flash = 40000000L
```

Dependências principais:

```text
LovyanGFX
LVGL
```

As versões devem ser fixadas explicitamente.

---

# Build

Com PlatformIO instalado:

```powershell
python -m platformio run
```

Upload:

```powershell
python -m platformio run --target upload
```

Monitor serial:

```powershell
python -m platformio device monitor
```

---

# Ordem de desenvolvimento

O desenvolvimento deverá seguir milestones incrementais.

## M1 — Display

```text
ST7789
320×240
LovyanGFX
```

## M2 — Touch

```text
XPT2046
calibração
```

## M3 — LVGL

```text
display flush
touch input
basic UI
```

## M4 — Footswitches

```text
FS1
FS2
debounce
momentary/latching
```

## M5 — UART

```text
921600 baud
loopback
stress test
```

## M6 — VoxLink

```text
framing
CRC
parser
HELLO
heartbeat
```

## M7 — VoxP4 integration

```text
capabilities
state snapshot
parameters
```

## M8 — Performance UI

```text
effects
meters
pitch
footswitch feedback
```

## M9 — Presets

```text
list
load
save
rename
```

## M10 — Hardening

```text
reconnect
error handling
stress tests
performance
```

---

# Prioridades do projeto

A ordem de prioridade é:

```text
1. Audio do VoxP4 nunca ser prejudicado
2. Footswitches responderem imediatamente
3. Comunicação ser confiável
4. Interface responder rapidamente
5. Estado mostrado ser correto
6. Interface ser visualmente agradável
```

Se houver conflito entre sofisticação gráfica e confiabilidade durante performance ao vivo, **confiabilidade vence**.

---

# Fora do escopo da v1

Inicialmente não fazem parte do projeto:

```text
Wi-Fi
Bluetooth
ESP-NOW
DSP no CYD
streaming de áudio
waveform contínua
FFT
spectrogram
multi-touch
SD para presets
firmware update do P4 pelo CYD
editor desktop
```

Esses recursos podem ser avaliados futuramente.

---

# Referências

Hardware e configuração CYD:

```text
https://github.com/ovelhaaa/beatCYDa
```

Arquivos particularmente importantes no BeatCYDa:

```text
src/CYD_Config.h
src/ui/LGFX_CYD.h
src/main.cpp
platformio.ini
README.md
touch_e_sd/exemplo_config.txt
```

---

# Documentação do projeto

Consulte também:

```text
AGENTS.md
docs/VOXP4_CYD_CONTROL_SURFACE_SPEC.md
```

`AGENTS.md` contém as regras de arquitetura e implementação para agentes e contribuidores.

A especificação em `docs/VOXP4_CYD_CONTROL_SURFACE_SPEC.md` contém os detalhes técnicos completos do controlador.

---

# Status

Projeto em fase inicial de desenvolvimento.

Próximo objetivo:

```text
ST7789
+
XPT2046
+
LVGL
+
2 footswitches
+
UART VoxLink
```

antes da integração completa com o DSP do VoxP4.
