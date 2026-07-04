# OTCS Pico Hello World

This is the first custom Pico 2 W firmware project for OTCS.

It builds a UF2 named `otcs_pico_hello.uf2`. Drag that UF2 onto the Pico while
it is mounted as `RP2350`, then watch USB serial output with Python miniterm.

## Build

From the repo root:

```powershell
$env:PICO_SDK_PATH = "C:\path\to\pico-sdk"
cmake -S firmware/hello_world -B build/pico-hello -G Ninja -DPICO_BOARD=pico2_w
cmake --build build/pico-hello
```

The UF2 will be created at:

```text
build/pico-hello/otcs_pico_hello.uf2
```

## Flash

1. Unplug the Pico.
2. Hold `BOOTSEL`.
3. Plug it back in.
4. Release `BOOTSEL` after the `RP2350` drive appears.
5. Drag `build/pico-hello/otcs_pico_hello.uf2` onto the `RP2350` drive.

The drive will disappear because the Pico reboots into the new firmware.

## Watch Serial Output

```powershell
python -m serial.tools.miniterm COM3 115200
```

Replace `COM3` with the COM port Windows assigns to the Pico.

Expected output:

```text
OTCS custom Pico hello world
Board: Raspberry Pi Pico 2 W
Message count: 1
```
