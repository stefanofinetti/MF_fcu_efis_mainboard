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
| even pin | pin 1 | +5 V (see the naming note below) |
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
| `Korry_*` | `RG` and `RW` have no value assigned in the schematics. Fitted parts are 220 Ω (green) and 91 Ω (white) |
| all backlight boards | Nets are named `+9V` but carry 5 V. At 9 V the white LEDs would draw 65 mA against a ~20 mA rating |
| `Mainboard` | Sheets `EFIS_LEFT` and `EFIS_RIGHT` point at files whose names are swapped. The result is correct; the filenames mislead |

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
