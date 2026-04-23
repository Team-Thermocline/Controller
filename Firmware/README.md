# Team Thermocline Chamber Firmware

This is built with the [Pico-SDK](https://github.com/raspberrypi/pico-sdk/blob/master/external/pico_sdk_import.cmake) and meant to run
on an [RP2040](https://pip-assets.raspberrypi.com/categories/814-rp2040/documents/RP-008371-DS-1-rp2040-datasheet.pdf?disposition=inline)

## To build

I made this using the self-contianed self-downloading `pico_sdk_import.cmake` so all you need to do is

```shell
mkdir build
cd build
cmake ../
make
```

## To load to your Pico

### Using picotool (recommended)

After building, picotool will be available in the build directory. To load the UF2 file:

```shell
# From the build directory
./_deps/picotool-build/picotool load thermocline_controller.uf2

# Or if picotool is installed system-wide:
picotool load -f -x thermocline_controller.uf2
```

You must be in bootsel mode!

Other useful picotool commands:
```shell
# List connected Picos
picotool info

# Reboot the Pico
picotool reboot

# Reboot into BOOTSEL mode
picotool reboot -u

# Load and reboot
picotool load -f thermocline_controller.uf2

# Verify what's loaded
picotool info
```

You can also drag and drop the uf2 to the pico's startup filesystem.

## State and control diagrams

Mermaid charts derived from the code directly.

### Chamber control FSM

```mermaid
stateDiagram-v2
  direction LR

  state "CHAMBER_STANDBY" as STBY
  state "CHAMBER_FAULT" as FLT
  state "CHAMBER_IDLE" as IDLE
  state "CHAMBER_HEATING" as HEAT
  state "CHAMBER_COOL_SLOW" as CSLOW
  state "CHAMBER_COOL_FAST" as CFAST
  state "CHAMBER_DEHUMIDIFY" as DEHUM

  [*] --> STBY : reset / boot

  STBY --> IDLE : arm idle

  note right of STBY
    STANDBY: all loads off.
    Clears FAULT except open TC,
    env sensor, compressor OC.
  end note

  IDLE --> FLT : fault
  HEAT --> FLT : fault
  CSLOW --> FLT : fault
  CFAST --> FLT : fault
  DEHUM --> FLT : fault

  note right of FLT
    FAULT: all loads off,
    internal fan GPIO low.
    Cleared fault returns to STANDBY
    until arm idle again.
  end note

  FLT --> STBY : fault cleared

  IDLE --> HEAT : need heat
  IDLE --> CSLOW : need slow cool
  IDLE --> CFAST : need fast cool

  HEAT --> IDLE : warm enough

  CSLOW --> IDLE : cool enough or cool disabled
  CFAST --> IDLE : cool enough or cool disabled

  CSLOW --> CFAST : ramp up to fast cool
  CFAST --> CSLOW : fast cool hysteresis

  CFAST --> DEHUM : air > 0 C, RH high, evap cold

  DEHUM --> IDLE : cool disabled or near-ambient
  DEHUM --> CFAST : dry dwell done

  note right of DEHUM
    Dehumidify: internal fan on,
    compressor off (chamber_outputs_apply_dehumidify).
    Enter from COOL_FAST when chamber air > 0 C,
    SHT35 RH > MAX_HUMIDITY_BEFORE_ENTERING_FREEZE (60 %RH default),
    and TDR1 evap <= THERMO_DEHUMIDIFY_ENTRY_EVAP_MAX_C (8 C default).
    Exit to COOL_FAST: elapsed >= 10 min AND
    |evap − 2 C| <= 0.75 C, OR elapsed >= 30 min.
    Exit to IDLE if active cooling is not allowed.
  end note
```

### Compressor contactor vs chamber mode

Nested FSM in `fsm_compressor()`; chamber states only request `compressor_want`.

```mermaid
flowchart LR
  subgraph req["Request from chamber_outputs_apply_*"]
    WON["compressor_want true: cool_fast only"]
    WOFF["compressor_want false: idle / heating / cool_slow / dehumidify / all_off"]
  end

  subgraph fsm["fsm_compressor (chamber_outputs.c)"]
    OFF["compressor_state OFF"]
    ON["compressor_state ON"]
    OFF -->|"want_on AND (first time OR off_duration >= MIN_COMPRESSOR_OFF_TIME_MS = 120000)"| ON
    ON -->|"want_off AND on_duration >= MIN_COMPRESSOR_ON_TIME_MS = 30000"| OFF
  end

  WON --> fsm
  WOFF --> fsm

  subgraph gpio["GPIO / interlock"]
    H["heat_gpio: heater AND NOT compressor_state"]
    C["condenser: cooling OR compressor OR headroom hyst"]
  end

  ON --> gpio
  OFF --> gpio
```

### Locked-rotor check and compressor overcurrent fault

```mermaid
sequenceDiagram
  participant T as Thermo tick (~100ms)
  participant O as chamber_outputs
  participant A as analog_task

  Note over O,A: When compressor GPIO is ON, <br> compressor_on and chamber_outputs_compressor_on_time() <br> track the on-time.

  O->>A: compressor_on true, t_on set
  loop each analog cycle
    A->>A: if compressor_on AND (now - t_on) >= COMPRESSOR_STARTUP_TIME_MS (500) <br> THEN if COMPRESSOR_LOAD > LOCKED_ROTOR_THRESHOLD_A (15) <br> finally -> FAULT_CODE_COMPRESSOR_OVERCURRENT
  end

  Note over A: During first COMPRESSOR_STARTUP_TIME_MS after on, no LR compare (gated startup window).

  O->>A: compressor off
  A->>A: if FAULT was COMPRESSOR_OVERCURRENT <br> AND (now - t_off) >= MIN_COMPRESSOR_OFF_TIME_MS (120000) <br> finally -> fault_raise(NONE)
```
