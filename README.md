ft_ascii_caster
================

Goal
----
This project was **unofficially** made as part of the 42 school cursus. It is an experiment on vibecoding and thus was entirely created by Copilot, with the developer only giving instructions and reviewing the code and program (see *Copilot Assistance* section).

Build
-----
From the project root:

```sh
make
```

Run
---
- Default (distance-based shading):

```sh
make run-distance
```

- Orientation mode (show wall face letters N/S/E/W):

```sh
make run-orientation
```

You can also run the binary directly:

```sh
./bin/ft_ascii_caster maps/classic.map
./bin/ft_ascii_caster -o maps/classic.map
```

Controls
--------
- `W`/`A`/`S`/`D`: move forward/left/back/right
- Arrow keys: rotate left/right
- `Q` or `Esc`: quit

Map format
----------
- A plain text `.map` file describing the top-down grid.
- Allowed characters:
	- `1` = wall
	- `0` = empty space
	- `N`, `S`, `E`, `W` = player start position and initial facing direction
- The parser is strict: the map must be rectangular, contain exactly one
	player, have no empty lines inside, and be closed by walls (edges must be `1`).
	On any error the program prints `Error: <message>` and exits after freeing
	resources.
- Minimal map example:
	```
	1111111111
	1000000001
	1011001101
	10000N0001
	1111111111
	```

Files of interest
-----------------
- `src/map_parser.c` — strict `.map` parser
- `src/render.c` — raycaster, terminal handling, mini‑map and controls
- `include/ft_ascii_caster.h` — shared types and prototypes

Notes
-----
- The renderer supports two display modes: distance‑based shading and
	orientation markers. Use the Makefile targets above to run either mode.
- The mini‑map is shown in the top‑left corner and updates in real time.

Copilot Assistance
------------------
This project was developed with assistance from GitHub Copilot acting as a
pair-programming AI assistant. Copilot helped scaffold the repository,
implement the `.map` parser, create the raycasting renderer and terminal
handling, and add bonus features (orientation-based wall characters and a
mini-map). All changes were reviewed and adjusted to comply with the project
constraints (pure C, allowed syscalls, strict compile flags, and no external
graphics libraries).

Why Copilot was used
- Speed up routine scaffolding (Makefile, folders, headers) so development can
	focus on the core algorithms.
- Provide implementation guidance and reduce boilerplate for terminal/raw-mode
	handling, DDA raycasting, and simple ASCII shading strategies.

How Copilot was used
- Copilot produced code suggestions and complete function implementations which
	were integrated and then reviewed manually for correctness, safety, and
	compliance with the assignment rules.
- The maintainer reviewed, tested, and adjusted the code; responsibility
	for final correctness and adherence to the subject remains with the human
	developer.

Notes on provenance and verification
- All generated code was run locally (compile and brief runtime checks) and
	adjusted to fix warnings, link the math library, and ensure correct terminal
	behavior.
