# M6 — VoxLink client on the CYD

The CYD now runs a real VoxLink v1 client. The ESP32-P4 is the authority; the
CYD is intent + display. UART transport is implemented but the physical link is
**not** verified (P4 UART GPIO assignment is still pending).

## Architecture

```
LVGL UI (UI task)                VoxLink client task (core 1)
-----------------                ----------------------------
ui_emit_action                   Serial2 RX -> Parser -> VoxLinkClient
  -> ui_set_parameter_local  --intentQueue-->  set_parameter
  -> ui_set_effect_enable_local                        |
ui_app_run                       Serial2 TX <-- take_tx  |
  <- ui_apply_*_authoritative <--uiEventQueue-- events  <+
```

* `src/voxlink/VoxLinkProtocol.h`, `VoxLinkCrc`, `VoxLinkCodec`, `VoxLinkParser`,
  `VoxLinkClient` are **pure C++** (no Arduino/LVGL) and run under
  `pio test -e native`.
* `src/voxlink/VoxLinkGlue.cpp` is the only Arduino/FreeRTOS file. It owns the
  single `VoxLinkClient` from one task and never touches LVGL.
* `src/voxlink/VoxLinkUi.h` exposes `voxlink_param_supported()` to the UI.

## Tasking and queues

* One UART task on core 1 (priority 4, 4096 bytes stack). It drains intents,
  reads `Serial2`, runs `client.tick()`, writes `Serial2`, drains client events
  into the UI event queue, and publishes a capability/status snapshot.
* `intentQueue` (16) UI -> client task.
* `uiEventQueue` (32) client task -> UI.
* Capability/status snapshot is copied under a spinlock for UI reads.
* No dynamic allocation, no `std::vector`/`std::string` on the hot path.

## State machine

`Disconnected -> HelloSent -> CapsReceiving -> StateReceiving -> Active`.
Link status is only `true` in `Active`. Any handshake failure returns to
`Disconnected` with a 250/500/1000/2000 ms backoff (reset on `Active`).

## Handshake / boot flow

```
Disconnected
  -> HELLO
  -> HELLO_ACK (major version checked; mismatch => disconnect)
  -> CAPS_REQUEST
  -> CAPS_BEGIN / CAPS_PARAM x N / CAPS_END
  -> GET_STATE
  -> STATE_BEGIN / STATE_PARAM x N / STATE_END
  -> Active
```

The UI link indicator stays disconnected until the snapshot is complete.

## Capability model

`CAPS_BEGIN` carries the protocol version, capability bits, sample rate, block
size and harmony voice count. `CAPS_PARAM` stores a fixed array (64) of
`{id, type, flags, min, max, default, step}`. Runtime truth is CAPS, never the
compiled parameter count (49). Effect Editor skips parameters that are not
advertised; `set_parameter` for an unsupported id is dropped locally (no wire
traffic).

Effect enables map to their real IDs (`VOXP4_PARAM_HARMONY_ENABLE`,
`REVERB_ENABLE`, `DELAY_ENABLE`, `HARMONY_LIMITER_ENABLE`); LIMITER is the
harmony bus limiter, never the master ceiling.

## Snapshot flow

`STATE_BEGIN(revision,count)` starts a temporary fixed buffer. `STATE_PARAM`
entries are buffered only. `STATE_END(revision)` requires begin==end revision
and count match, then applies all values as authoritative events (the UI applies
them in one tick before rendering). No partial state is shown.

## Local pending vs authoritative

* Boot values: `LocalDefault`.
* Local edit: `ui_set_parameter_local()` -> `LocalPending` + a wire intent.
* P4 value (`STATE_PARAM`, `PARAM_VALUE`, `PARAM_CHANGED`):
  `ui_apply_parameter_authoritative()` -> `Authoritative`.
* Each parameter keeps a `parameterAuthoritativeValues[]` for rollback.
* Effect enables use the same model (`effectAuthority`, `effectAuthoritative`).

Only `PARAM_CHANGED` promotes a value; `ACK` means protocol-level acceptance
only.

## Slider coalescing

Continuous `Float32` parameters are coalesced client-side: latest value wins,
flushed every ~33 ms (30 Hz). Bool/enum/int are sent immediately. The wire tag
comes from CAPS, so `floats` are never sent for discrete parameters.

## NACK / QUEUE_FULL

* `NACK` for a SET: emit `ParamRevert`; the UI rolls back to the last
  authoritative value.
* `NACK(QUEUE_FULL)`: keep the latest desired value and resend in the next
  coalescing window (no avalanche).
* Request timeout (500 ms): drop pending and roll back the same way.

## Reconnect

On heartbeat timeout (3 s) the client disconnects, emits `LinkDown` (link only,
never DSP/UI state changes) and restarts `HELLO -> CAPS -> GET_STATE`. Locally
pending values from the offline window are **not** replayed; the P4 snapshot
wins.

## Revision semantics

`state_revision` uses wrap-safe comparison `(int32_t)(a - b) > 0`. Stale
`PARAM_CHANGED` (older revision, not a direct response to our SET) is ignored. A
no-op confirmation with the same revision is still accepted because it matches
the pending request.

## Counters / debug

Counters: `frames_rx/tx`, `bytes_rx/tx`, `crc_errors`, `length_errors`,
`version_errors`, `unknown_messages`, `unknown_params`, `parse_errors`,
`timeouts`, `nacks`, `queue_full`, `reconnects`, `tx_drops`. The System screen
shows link state, baud, RX/TX and error counters, refreshed at 2 Hz.
`DEBUG_BUILD` may log handshake milestones; release logs nothing per frame.

## Offline behavior

Without a P4 the UI starts with descriptor defaults, link shows OFF, and the
user may navigate/edit. Local edits are visual only and are **not** replayed on
connect; the first `GET_STATE` replaces everything.

## Limitations / pending

* Physical CYD <-> P4 link NOT tested: `CONFIG_VOXLINK_UART_TX_GPIO` /
  `_RX_GPIO` are `-1` in voxP4 pending schematic verification. CYD UART pins are
  the pre-existing `VoxUartTx=18` / `VoxUartRx=19` (SD bus reused); 921600 8N1,
  manual 460800 fallback, no auto-baud.
* Presets/global bypass/MIDI/scenes/telemetry are capability-gated and not
  implemented (the production P4 does not advertise them).
* Hardware acceptance is separate: P4 GPIO verified + wires + common GND +
  observed HELLO..Active.
