# Archived schematics

Files kept here are **not part of the FCU_Mainboard_v3 design**. They are not
referenced by the root sheet and are excluded from ERC, DRC, the netlist and
the BOM. Nothing in the fabricated board depends on them.

## backlighting.kicad_sch

Panel backlighting for the FCU + EFIS unit. It is **not connected to the
mainboard**: the backlight has its own dedicated circuitry and is not driven by
MobiFlight, so brightness cannot be controlled from the simulator.

Kept because the design may be revisited if backlight control is ever brought
under MobiFlight. If that happens it will be redesigned from scratch rather
than wired into the current board, so treat this file as a reference for the
existing circuit, not as a starting point.

## Removed, recoverable from git history

Two more orphan sheets were deleted rather than archived, both superseded
leftovers from v2:

- `EFIS.kicad_sch` — the v2 combined EFIS sheet. Its reference designators
  clash with the live sheets while meaning something else (J16 was
  `L_EFIS_FD`, it is now `R_FD`), and its global labels use an older naming
  scheme (`L_EFIS_FD_BTN`, `L_EFIS_RNG_160`, ...) that no longer exists.
  Superseded by `efis_left.kicad_sch` and `efis_right.kicad_sch`.
- `multiplexers.kicad_sch` — U3/U4 as CD74HC4067 breakout modules. The board
  now uses bare `74xx:CD74HC4067M` in SOIC-24W.

Both were removed in the commit that created this file; `git log --diff-filter=D
-- "Kicad Files/EFIS.kicad_sch"` will find them.
