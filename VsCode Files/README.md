# SF FCU — firmware

MobiFlight custom device that drives the seven OLED displays of an A320
FCU/EFIS panel: the five FCU windows and the two EFIS barometer displays.

Buttons, encoders and LEDs are plain MobiFlight and are configured in the
Connector. They do not go through this firmware.

## What this tracks

The reference is **MobiFlight's own scaffolding**: the project layout,
`platformio.ini`, the build scripts and `MFCustomDevice.*` follow
[MobiFlight/CommunityTemplate](https://github.com/MobiFlight/CommunityTemplate),
and `custom_core_firmware_version` is kept at the core firmware release the
template points to — currently **3.1.4**. `src/` is not in the repository;
`get_CoreFiles.py` clones the core on the first build.

The display code came from Gagagu's
[A320 EFIS/FCU display](https://github.com/gagagu/Mobiflight-A320-Efis-Fcu-Display-with-ESP32)
by way of [elral/MF_FCU_EFIS_OLEDs](https://github.com/elral/MF_FCU_EFIS_OLEDs)
v1.0.1. `FCU_EFIS.cpp`, `OLEDInterface.h` and the fonts are theirs. Following
elral is *not* a goal — following the MobiFlight core is.

## What differs from the template

| | |
|---|---|
| Device type | `SF_FCU`, so it installs alongside the upstream one |
| EFIS channels | left and right are the other way round from upstream |
| `SERIAL_RX_BUFFER_SIZE` | 256, not the template's 96 — see below |
| Targets | Mega only |

### Multiplexer channels

The seven displays hang off a PCA9548A, one display per channel:

| Channel | Display |
|---|---|
| 0 | EFIS right — barometer |
| 1 | EFIS left — barometer |
| 2 | SPD / MACH |
| 3 | HDG / TRK |
| 4 | HDG-V/S / TRK-FPA annunciator |
| 5 | ALT |
| 6 | V/S — FPA |

The channel numbers live in `FCU_EFIS.h`. Change them there if the displays
are wired to different channels.

### Serial receive buffer

A display update blocks the loop for roughly 28 ms — 5 ms of `delay()` for the
multiplexer switch plus a 1 KB frame at 400 kHz. At 115200 baud more than 300
bytes can arrive in that window, so the template's 96-byte receive buffer
would drop characters. Do not lower it.

### Footprint

The ATmega2560 has 8 KB of RAM and MobiFlight reserves most of it up front:
1600 bytes for the device arena (`MF_MAX_DEVICEMEM`) and 1000 for the input
names (`MEMLEN_NAMES_BUFFER`). Those two are sized for the Connector config,
not for this code, and are the wrong place to economise.

What this project does instead:

* **`build_unflags` turns off the device families the panel cannot have** —
  segment displays, character LCD, steppers, servos, analog inputs and both
  shift registers. Input multiplexers stay on; the panel uses them. Buttons,
  encoders and outputs are core and always present. This is worth 181 bytes
  of RAM and about 14 KB of flash. To get one back, delete its line.
* **No `String` anywhere.** The values from the Connector live in fixed
  buffers, so a redraw allocates nothing and the heap cannot fragment. The
  only allocation left in the whole firmware is Adafruit's 1 KB frame buffer,
  taken once at startup.

Static RAM actually rose by 18 bytes when the `String`s went, because nine
6-byte objects became nine 8-byte buffers. The point was never the 18 bytes:
it was that each of those objects also held a heap block, and three more were
allocated and freed on every single redraw.

## I2C address and display type

The custom device takes the address of the PCA9548A, which the multiplexer's
A0/A1/A2 pins set: all low is 0x70.

The address also picks the display driver, which is easy to miss:

* **even** address (0x70) → **SH1106**
* **odd** address (0x71) → **SSD1306**

So an SSD1306 needs the multiplexer strapped to an odd address and the custom
device set to match. The shipped `SF FCU.mfmc` is set to 0x70, i.e. SH1106.

## Building

```
pio run -e SF_FCU_mega
```

That produces version `0.0.1`. For a real one set `VERSION` — the build
stamps it into both the firmware filename and `board.json`, which have to
agree or MobiFlight will not find the firmware:

```
VERSION=1.1.0 pio run -e SF_FCU_mega
```

`copy_fw_files.py` only stamps the version when `_build/` does not yet exist,
so delete `_build` and `_dist` before building a different version.

The result is an installable ZIP in `_dist/`. Extract it into the `Community`
folder of your MobiFlight installation, then flash from the Connector.

`documents/` holds Gagagu's original drawings, kept for reference.

## Known gaps, inherited from upstream

* MessageID −1 (Connector closing) and −2 (power saving, after the 600 s in
  the `.mfmc`) are not handled, so the displays keep showing the last values
  instead of going dark. On OLEDs that is a burn-in risk.
* The Mach zero-padding in `updateDisplayFcuSpd()` writes past the end of the
  `String` and silently does nothing.
* The zero-padding elsewhere relies on `String::operator[]` returning a zeroed
  dummy when read out of range. It works, but on an implementation detail.
