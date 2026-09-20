# A320 Glareshield — FCU + EFIS

This repository contains the complete glareshield unit: the FCU in the centre
and the two EFIS control panels either side, driven by
[MobiFlight](https://www.mobiflight.com/). The two outer side panels are not
built — three rarely used buttons did not justify the plastic.

The unit sits on a desk, is powered by its own mains adapter and connects to
the PC over USB, so it can be unplugged and moved.

* STL Files for 3D Printing, along with the .3mf files for multicolor printing
* SKP Files from SketchUp, if you want to edit dimensions or change anything
* Kicad files for editing the PCBs or changing components

## Features
- If you decide to not modify anything, you can just print the various 3D components and order the PCBs from JLCPCB.
- You will need a lot of materials, if you open the Kicad Projects you can obtain the BOM directly from them.
- You will need to build your own custom firmware (source provided) or you can use the one in the ZIP file inside the \_dist directory in VsCode Files

---

## Boards

| Folder | Project | Qty | Role |
|---|---|---|---|
| `Mainboard/` | `FCU_Mainboard_v3` | 1 | ATmega2560 + CH340G, 7 OLEDs behind a PCA9548A I²C mux, 4× CD74HC4067 analogue mux for the buttons, 20 annunciator outputs. 250 × 100 mm, 2 layers |
| `PSU/` | `9V_Distribution` | 1 | 9 V in → LM2596S-5 → 5 V out on three screw terminals |
| `Backlighting_Dimmer/` | `Backlighting_PCB` | 1 | NE555 PWM dimmer, potentiometer controlled, IRLIZ44N low-side switch, three outputs |
| `Backlighting_LEDModule/` | `Backlighting_LEDModule_PCB` | 3 | Passive fan-out: one input → twenty 2-pin outputs. Three of them only to keep the LED strip wiring short |
| `Korry_Large/` | `KorryLargePCB_SMD` | — | Tactile switch + green annunciator LED + white backlight LED |
| `Korry_Small/` | `KorrySmallPCB_SMD` | — | Same, smaller outline |

## Power architecture

```
 mains adapter 9 V / 3 A
        │
        └──► PSU  J1 ──► D2 ──► D1 ──► LM2596S-5 ──► L1 ──► 5 V ──┬──► J2 ──┐
                                                                   ├──► J3 ──┤  three taps,
                                                                   └──► J4 ──┘  same node
                                                                                    │
 PC ──USB──► Mainboard                                    Backlighting_Dimmer ◄──────┘
   (powered USB3 hub)                                            │
        │                                                        └──► 3 × LEDModule
        └──► 20 × Korry  (green annunciator LED + button)                  │
                   └──────────── white backlight LED ◄──────────────────────┘
                                 + 6 LED strip segments
```

Two independent supplies that share only the PC ground through the USB shield.
The mainboard runs off USB; everything that draws real current runs off the
mains adapter.

## Interconnect

### Mainboard → Korry (`BTN+LED`, 3-pin)

| Mainboard `J*` | Korry `J1` | Signal |
|---|---|---|
| pin 1 | pin 3 | GND |
| pin 2 | pin 2 | annunciator LED, driven by an MCU pin through the 220 Ω on the Korry board |
| pin 3 | pin 1 | button to GND, read through a CD74HC4067 |

> **The pinouts are mirrored.** Mainboard pin 1 is GND while Korry pin 1 is the
> button. A straight-through 3-wire cable would swap GND and the button line;
> the connector is plugged the other way round, which crosses 1↔3 and leaves
> pin 2 in the middle. Worth aligning in a future revision.

### LEDModule → Korry (`BK_LED`, 2-pin)

| LEDModule `J1`/`J2` | Korry `J2` | Signal |
|---|---|---|
| even pin | pin 1 | +5 V |
| odd pin | pin 2 | switched return, through the 91 Ω on the Korry board |

The same mirroring applies here.

### Backlighting_Dimmer → LEDModule (`BOARD_CONN`, screw terminal)

| Dimmer `J12`/`J13`/`J14` | LEDModule `J11` | Signal |
|---|---|---|
| pin 1 | pin 2 | +5 V, always present |
| pin 2 | pin 1 | return, switched to GND by the IRLIZ44N |

### PSU → consumers

`J2`, `J3` and `J4` are three taps on one node, not three independent lines.
There is no per-output fusing: the only protection is the 1 A polyfuse on the
9 V input, which caps the whole board at roughly 1.4 A at 5 V.

## Current budget

Calculated where the resistors are known, estimated elsewhere.

| Load | Rail | Current |
|---|---|---|
| 7 OLEDs | +3V3, from +5V through the AMS1117 | 105–140 mA |
| Green annunciators, 11 lit worst case (220 Ω) | driven by MCU pins | ~130 mA |
| ATmega2560 + CH340G + muxes + power LED | VIN / +5V | ~55 mA |
| **Mainboard total, from USB** | | **290–340 mA** |
| 20 white backlight LEDs (91 Ω) | backlight 5 V | 418 mA |
| 6 LED strip segments, 3528 | backlight 5 V | 360–720 mA |
| **Backlighting total, from the adapter** | | **780–1140 mA** |

The adapter supplies 27 W and the unit uses under 7 W. The binding constraint
is the PSU board's own 1 A input polyfuse, not the adapter.

## Known issues

| Board | Issue |
|---|---|
| `PSU` | **The LM2596 has no catch diode.** A non-synchronous buck requires a Schottky from the switch node to ground; the only diode on the board is in series with the input. The part runs out of spec and the effect worsens with load |
| `PSU` | `C1` 100 µF and `C2` 220 µF are on 1206 footprints. Those values are implausible as ceramics, and an LM2596 wants some ESR on its output for loop stability. Check what is actually fitted |
| `PSU` | The 1 A input polyfuse caps the board at about a third of what the adapter can deliver |
| `Backlighting_Dimmer`, `Backlighting_LEDModule` | The screw terminal footprint `TerminalBlock:TerminalBlock_bornier-2_P5.08mm` no longer exists in the KiCad library. KiCad 10 ships no 2-pin 5.08 mm terminal block at all, so there is nothing to point it at: retargeting it would change the pad geometry of a board that is already made. The copy embedded in the board is correct and is what gets manufactured; only the library link dangles |
| `Korry_Large` | Four `+` and `-` markers on the back silkscreen are not mirrored. Both glyphs are symmetric so nothing reads wrong, and since they are justified `left bottom`, adding the mirror flag would shift them by about a glyph width. Left alone on purpose |

## Libraries you will need

All six projects are KiCad 10. Every board reports 0 unconnected items and 0
schematic parity issues, but ERC and DRC still flag missing libraries: the
symbols are embedded in the files so nothing is lost, only the links dangle.
Install these from the Plugin and Content Manager to clear them:

| Library | Used for |
|---|---|
| Alternate KiCad Library | `PCM_LED_AKL`, `PCM_LED_SMD_AKL`, `PCM_Diode_Schottky_AKL`, `PCM_Diode_SMD_AKL` |
| Digi-Key library | `dk_Tactile-Switches`, `dk_Clock-Timing-Programmable-Timers-and-Oscillators` |
| SL Screw Terminal | `PCM_SL_Screw_Terminal` |

The remaining `lib_footprint_mismatch` warnings are footprints that changed
between the KiCad version each board was drawn in and version 10. Running
`Update Footprints from Library` clears them, but it does change pad and
silkscreen geometry, so it is worth reading the diff before doing it on a
board that is already made.

## Gotchas

- **`F8` in Pcbnew clears `exclude_from_pos_files` on the seven mounting
  holes**, which puts them into the pick-and-place file. Re-check before
  generating production output.
- The two 0.4102 mm segments on `+3V3` next to the PCA9548A pad are a
  deliberate neck. Widening them breaks clearance.
- `VIN` has its own netclass with 0.1 mm clearance. Moving it to the `5V`
  class, which uses 0.2 mm, produces about 70 violations against existing
  copper.
- Production files are generated by the
  [Fabrication Toolkit](https://github.com/bennymeg/Fabrication-Toolkit)
  plugin and are not versioned.

---

## Getting Started
### Prerequisites
- Software required:
   * [KiCad](https://www.kicad.org/) — version 10 or later, the projects have been migrated
   * [SketchUp](https://www.sketchup.com)
   * [VsCode]
   * [MobiFlight]

- Any libraries or components used.

### Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/stefanofinetti/MF_A320_Glareshield.git
   ```

## License

This code and all the items are release under GPLv3.0 license. Feel free to use as you wish as long as you redistribute the source code.
It would be nice a mention, though, if you use this work. I invested a good amount of hours just to make it for the amazing MobiFlight community.

Pull requests that fix any of the issues listed above are very welcome.

## Acknowledgements

 This project couldn't have seen the light without the excellent work, and kind support, of [GaGagu](https://github.com/gagagu) and [ElRal](https://github.com/elral). Their work is amazing, so please have a look at their repositories
