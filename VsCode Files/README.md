# SF FCU — firmware

MobiFlight custom device that drives the seven OLED displays of the FCU/EFIS
mainboard: the five FCU windows and the two EFIS barometer displays.

Everything else on the board — buttons, encoders, LEDs, the Korry units — is
plain MobiFlight and is configured in the Connector. It does not go through
this firmware.

## Lineage

This is [elral/MF_FCU_EFIS_OLEDs](https://github.com/elral/MF_FCU_EFIS_OLEDs)
v1.0.1, which in turn is Gagagu's
[A320 EFIS/FCU display](https://github.com/gagagu/Mobiflight-A320-Efis-Fcu-Display-with-ESP32)
ported to the MobiFlight community device API. All the display code is theirs.

Two things are changed here, and nothing else:

| What | Why |
|---|---|
| The device is named `SF_FCU` instead of `GAGAGU_FCU-EFIS` | so it can be installed next to the upstream one, and to match SF_RMP and SF_OVHD |
| `TCA9548A_CHANNEL_EFIS_LEFT` and `_RIGHT` are swapped | see below |

`FCU_EFIS.cpp`, `OLEDInterface.h` and `MFCustomDevice.h` are byte-identical to
upstream, so a diff against a newer release stays readable. Keep it that way.

## Multiplexer channels

The displays hang off a PCA9548A (`I2C_1` on the mainboard). Each channel is
one connector:

| Channel | Connector | Display |
|---|---|---|
| 0 | J1 `R_OLED` | EFIS right — barometer |
| 1 | J2 `L_OLED` | EFIS left — barometer |
| 2 | J3 `SPD_OLED` | SPD / MACH |
| 3 | J4 `HDG_OLED` | HDG / TRK |
| 4 | J5 `H_T_OLED` | HDG-V/S / TRK-FPA annunciator |
| 5 | J14 `ALT_OLED` | ALT |
| 6 | J15 `VSPEED_OLED` | V/S — FPA |

J1 sits in the `R_` connector cluster at one end of the board and J2 in the
`L_` cluster at the other, so the left EFIS is on channel 1 and the right one
on channel 0 — the opposite of upstream, which is why those two constants are
swapped in `FCU_EFIS.h`.

## I2C address and display type

The address is set by SW1 on the mainboard: all switches open is 0x70, each
one closed pulls its address bit high.

The address also picks the display driver, which is easy to miss:

* **even** address (0x70) → **SH1106**
* **odd** address (0x71) → **SSD1306**

So an SSD1306 needs SW1 A0 closed and the custom device set to 0x71. The
shipped `SF FCU.mfmc` is set to 0x70, i.e. SH1106.

## Building

PlatformIO. `src/` is not in the repository: `get_CoreFiles.py` clones the
MobiFlight firmware core (tag `2.5.1`) on the first build.

```
pio run -e SF_FCU_mega
```

The build drops a ready-to-install ZIP in `_dist/`. Extract it into the
`Community` folder of your MobiFlight installation, then flash from the
Connector.

`SF_FCU_raspberrypico` builds too, but the mainboard carries a soldered
ATmega2560 — that target is inherited from upstream and untested here.

## Wiring

`documents/FCU_EFIS.png` is Gagagu's breadboard drawing and does **not**
describe this board. The wiring is the KiCad project in `Mainboard/`.

## Known gaps, inherited from upstream

* MessageID −1 (Connector closing) and −2 (power saving, after the 600 s in
  the `.mfmc`) are not handled, so the displays keep showing the last values
  instead of going dark.
* `MFCustomDevice::detach()` does not check `_initialized`.
* The Mach zero-padding in `updateDisplayFcuSpd()` writes past the end of the
  `String` and silently does nothing.
* Each display update blocks for roughly 28 ms — 5 ms of `delay()` for the
  channel switch plus the 1 KB frame at 400 kHz. A light test redraws all
  seven.
