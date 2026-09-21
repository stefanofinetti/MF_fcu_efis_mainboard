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
| `PSU/` | `9V_Distribution` | 1 | 70 × 80 mm. Two independent regulators off one 9 V input: an LM2596S-5 buck for the backlighting and a P78A05-1000 for the mainboard. See *PSU revision* below |
| `Backlighting_Dimmer/` | `Backlighting_PCB` | 1 | NE555 PWM dimmer, potentiometer controlled, IRLIZ44N low-side switch, three outputs |
| `Backlighting_LEDModule/` | `Backlighting_LEDModule_PCB` | 3 | Passive fan-out: one input → twenty 2-pin outputs. Three of them only to keep the LED strip wiring short |
| `Korry_Large/` | `KorryLargePCB_SMD` | — | Tactile switch + green annunciator LED + white backlight LED |
| `Korry_Small/` | `KorrySmallPCB_SMD` | — | Same, smaller outline |

## Power architecture

```
 mains adapter 9 V / 3 A
        │
        └──► PSU  J1 ──► D1 ──┬──► F2 ──► LM2596S-5 ──► L1 ──┬──► 5 V ──┬──► J2 ─┐
                              │   (2 A)     D2 ◄──┘          │          ├──► J3 ─┤
                              │             catch      C2 ‖ C4          └──► J4 ─┤
                              │                                                  │
                              └──► F3 ──► P78A05-1000 ──► J5 ──► Mainboard        │
                                 (0.5 A)   C5‖C6  C7‖C8          (clean 5 V)      │
                                                                                  │
 PC ──USB──► Mainboard                                    Backlighting_Dimmer ◄───┘
   (powered USB3 hub)                                            │
        │                                                        └──► 3 × LEDModule
        └──► 20 × Korry  (green annunciator LED + button)                  │
                   └──────────── white backlight LED ◄──────────────────────┘
                                 + 6 LED strip segments
```

Two supply chains that share only ground. The backlighting, which draws the
real current, runs off the mains adapter through the buck. The mainboard runs
off USB today; `J5` is there for the automatic selector that will let it take
the adapter instead whenever one is plugged in. That selector is not built
yet — see *Known issues*.

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

## PSU revision

The 9 V distribution board was reworked. The schematic and layout in this
repository are the revised version; the board physically in the glareshield is
still the old one.

| Change | Why |
|---|---|
| **`D2` added** — SS54 Schottky, cathode on the switch node, anode to ground | A non-synchronous buck has nowhere for the inductor current to go when the switch opens. Without an external diode it freewheels through the substrate diode of the LM2596 itself: roughly 0.4 W of extra dissipation inside the package on top of its own 0.7 W, and substrate injection the part is not specified for. This is why the board works and why it degrades |
| **`C4` added** — 220 µF radial electrolytic in parallel with `C2` | The LM2596 control loop needs some ESR on the output. `C2` is a ceramic, which has almost none. The RMP V5, same family of design but with electrolytics at its regulator, has never misbehaved |
| **`F2` 1 A → 2 A** (`MF-RHT100` → `MF-RHT200`) | At 1.14 A out the input draws about 0.75 A. Derated for the temperature inside a printed case, a 1 A polyfuse holds under 0.8 A — too close to nuisance tripping. Pads are identical, so the swap costs nothing in layout |
| **Power section re-laid out** | `U4` pin 2, the cathode of `D2` and pin 1 of `L1` now sit on one straight line at y = 64.95, and `C1` sits 4 mm under the VIN pin. The commutation loop went from *nonexistent* to about 4 mm of forward track each side |

### The mainboard branch

The mainboard needs a 5 V source that does not depend on the USB hub. It does
**not** take it from `J2`/`J3`/`J4`: those are one node with the backlighting
on it, PWM-switched by the dimmer's IRLIZ44N in steps of up to 1.1 A. Putting
the MCU and seven OLEDs on that rail would feed them those steps.

Instead a second, independent regulator, tapped from D1's cathode: the reverse
protection is shared, the fuse is not, so a fault on one branch cannot take
down the other. `F3` is 0.5 A for the branch's ~215 mA at 9 V. `U1` is the same
P78A05-1000 module used on RMP V5, with 100 nF + 100 uF each side as there.

The board grew from 50 × 70 to **70 × 80 mm** to make room. The whole
regulator chain lives in the new strip at x 150..170, so the buck section
keeps its original placement and its original routing, and every track stays
on the front: the bottom layer is an uninterrupted ground plane.

Every track is on the front. **The bottom layer is an uninterrupted ground
plane**, so the return current of each loop runs directly beneath its outward
track and the enclosed area stays small. Stitching vias next to the ground
terminals of `C1`, `C2`, `C4` and the anode of `D2` are tied to their pads by
short tracks rather than through the thermal spokes.

Not changed: the outline, the four mounting holes and the positions of `J1`
to `J4`, so the board still drops into the same case with the same wiring.

## Mainboard selector

The schematic half is done. `U8` is a TPS2115A power mux: `IN1` takes the
clean 5 V from the PSU's `J5`, `IN2` takes USB `VBUS`, and `OUT` becomes the
board's `+5V`. `D0` is tied to ground and `D1` comes from a 100k/100k divider
on the incoming rail, which makes the choice deterministic rather than
"whichever input happens to be higher": the external supply wins whenever it
is above about 4 V, and the board falls back to USB below that.

The part has no body diodes across its switches, so nothing can flow back
into the USB port when the board runs on the adapter. `ILIM` is set to about
0.8 A by `R34`. `STAT` is left unconnected; it is an open-drain output that
goes low on `IN1`, if an indicator is ever wanted.

`P1` pin 1 and `R14`, the D+ pull-up, moved off `+5V` onto the new `VBUS`
net. `JP1`, `VCC` and the ISP path are untouched: `JP1` still chooses whether
`VIN` comes from the board rail or from the programmer.

The symbol lives in `Mainboard/TPS2115A.kicad_sym` with a project-local
`sym-lib-table`, so the repository is self-contained.

**What is missing is the board.** Counting tracks as well as footprints, the
mainboard has no free rectangle big enough for the cluster anywhere near the
power section: the largest gaps are about 14 x 12 mm at the bottom-left
corner and a 42 x 7 mm strip around x 38..80, y 110..117 — neither takes the
screw terminal and the electrolytic. Fitting this needs either a wider board
or a relayout of the bottom band.

## Known issues

| Board | Issue |
|---|---|
| `PSU` | **Boards made from an earlier revision have no catch diode.** If you already built one, do not run it as it is: either remake it from the current files, or fit a 3–5 A / 40 V Schottky on the back, cathode to `L1` pin 1 and anode to the ground plane about 4 mm away, near the via at (129.0, 71.35). Check continuity to `J1` pin 2 with a meter before soldering |
| `PSU` | `C1` sits across the 9 V input. Its voltage rating needs to be 25 V: a 1206 MLCC of that capacitance is typically rated 6.3 V or 10 V, which is at or over the limit and loses most of its capacitance to DC bias long before that |
| `Mainboard` | **The automatic supply selector is on the schematic but not on the board.** `U8` (TPS2115A), `J13`, `C25`–`C27` and `R32`–`R34` exist in `pwr_conn.kicad_sch` and ERC passes, but the PCB does not carry them yet, so schematic parity reports eight missing footprints. The board has no contiguous free area left for them near the power section — see *Mainboard selector* below |
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

- **The `PSU` ground zone needs refilling.** The layout was edited outside
  KiCad, so the stored fill is the one computed for the old component
  positions. Open the board and press `B` before DRC or before generating
  production files. Until you do, DRC reports about 56 clearance, hole and
  solder-mask violations that are all the stale pour and nothing else.
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
