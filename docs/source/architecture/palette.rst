Palette System
==============

OpenOMF uses paletted rendering. Every pixel in every sprite is stored as an 8-bit
palette index (0-255). At render time, these indices are resolved to RGB colors via a global
palette. The palette itself can be manipulated per-frame by stacking **palette transforms** to
produce effects like tinting, darkening, and cross-fading.

The internal palette has been extended from the original VGA 256-color limit to **1024 colors**
to support features like expanded HAR color gradients. Game data files store colors as 6-bit
per channel (0-63); these are converted to 8-bit at load time.

Palette Layout
--------------

The 256-color base palette is divided into the following zones:

==============  ===============================================
Index Range     Purpose
==============  ===============================================
0x00            Always black (transparency / background)
0x01 - 0x2F     Player 1 HAR colors (3 shades x 16 hues)
0x30            Always black (transparency / background)
0x31 - 0x5F     Player 2 HAR colors (3 shades x 16 hues)
0x60 - 0x9F     Background / arena colors (64 colors)
0xA0 - 0xF9     Shared object colors (projectiles, effects, hazards)
0xFA - 0xFF     Menu UI colors (6 colors)
==============  ===============================================

On scene load, the entire palette is set from the BK file, then menu colors are written to
indices 250-255, index 0 is forced to black, and HAR colors overwrite 0x01-0x5F at arena start.

The 64-color background block (0x60-0x9F) can be swapped from BK palette variants to change
the arena's look (e.g. the desert arena cycles palettes between rounds to simulate time of day).

Index 255 pulses through a green gradient on a 16-tick cycle, producing the animated
selected-menu-item glow.

HAR Colors
----------

Each player gets a 48-index block divided into 3 **shades** of 16 **hues**::

    Player 1:  [0x00 .. 0x0F] shade 0   (index 0 is always black, so effectively 0x01-0x0F)
               [0x10 .. 0x1F] shade 1
               [0x20 .. 0x2F] shade 2

    Player 2:  [0x30 .. 0x3F] shade 0   (index 0x30 is always black, so effectively 0x31-0x3F)
               [0x40 .. 0x4F] shade 1
               [0x50 .. 0x5F] shade 2

HAR sprite data always uses indices in the 0-47 range. Each object has ``pal_offset`` and
``pal_limit`` fields; the fragment shader shifts any index within the limit by the offset.
Player 1 uses offset 0, player 2 uses offset 48. Projectiles and scrap pieces inherit the
parent HAR's offset/limit.

.. graphviz::

   digraph pal_offset {
       rankdir=LR;
       node [shape=box, style=filled, fontname="sans-serif", fontsize=10];
       edge [fontname="sans-serif", fontsize=9];

       sprite [label="Sprite index\n0x05", fillcolor="#b3d9ff"];
       add [label="+ pal_offset", shape=ellipse, fillcolor="#e8e8e8"];
       p1 [label="Player 1: 0x05\n(offset 0)", fillcolor="#b3ffb3"];
       p2 [label="Player 2: 0x35\n(offset 48)", fillcolor="#ffcccc"];

       sprite -> add;
       add -> p1;
       add -> p2;
   }

Each pilot has three color choices (primary, secondary, tertiary), with values 0-16. Values
0-15 select a 16-color block from ``ALTPALS.DAT`` palette 0; value 16 uses the pilot's custom
palette from ``PLAYERS.PIC``. The three choices map to the three shade blocks. For cutscenes,
the 3x16 color blocks are expanded to 3x32 by interpolation to provide smoother gradients.

Palette Effects
---------------

.. graphviz::

   digraph palette_pipeline {
       rankdir=LR;
       node [shape=box, style=filled, fillcolor="#e8e8e8", fontname="sans-serif", fontsize=10];
       edge [fontname="sans-serif", fontsize=9];

       base [label="Base\npalette", fillcolor="#b3d9ff"];
       copy [label="Copy to\ncurrent", shape=ellipse];
       obj [label="Object\ntransforms\n(HAR effects,\nBK tag effects)"];
       scene_cf [label="Scene\ncross-fade"];
       scene_t [label="Scene\ntransforms\n(arena crossfade)"];
       gpu [label="Upload dirty\nranges to GPU", fillcolor="#b3ffb3"];

       base -> copy -> obj -> scene_cf -> scene_t -> gpu;
   }

Up to 8 transforms can be stacked per frame. Each transform marks which palette indices it
modified via a damage tracker, enabling partial GPU uploads. The available operations are:

- **Tint** (``vga_palette_tint_range``): Brightness-weighted blend toward a reference color.
- **Mix** (``vga_palette_mix_range``): Linear interpolation toward a reference color.
- **Darken** (``vga_palette_darken``): Scale all colors toward black.
- **Light range** (``vga_palette_light_range``): Brightness-weighted blend toward a gray level.
- **Multiply** (``vga_state_mul_base_palette``): Scale a range by a float, applied directly to
  the base palette rather than as a stacked transform.

These are driven by animation script tags. HAR animations use tint/mix with a three-phase
envelope (fade in, sustain, fade out) for hit flash effects. Background animations drive
scene-wide palette shifts over time. On the Stadium arena, positional lighting blends each
HAR's palette toward a gray value based on distance from center.

Remap Tables
------------

The game uses **19 remap tables**, each a 256-entry lookup that maps one palette index to
another: ``new_index = table[old_index]``. At load time each table is expanded to 1024 entries
and stored as a GPU texture (entries beyond 255 map to themselves).

A remap table can do anything expressible as an index-to-index mapping. For example, a table
could shift all HAR colors toward darker hues (for a shadow effect), swap one color range for
another (for a glow), or collapse multiple indices onto the same target (for a silhouette).
The tables are authored per-arena in the BK files, so each arena can define its own visual
style for these effects.

Remaps are applied in the fragment shader after the palette index is determined. The effect
selects which table to use and how many **rounds** to apply. With one round, the index is
looked up once. With multiple rounds, the result is fed back through the same table repeatedly,
pushing the color further along:

.. graphviz::

   digraph remap {
       rankdir=LR;
       node [shape=box, style=filled, fontname="sans-serif", fontsize=10];
       edge [fontname="sans-serif", fontsize=9];

       input [label="Pixel index", fillcolor="#b3d9ff"];
       table [label="Remap\ntable[index]", shape=ellipse, fillcolor="#e8e8e8"];
       output [label="Final index", fillcolor="#b3ffb3"];

       input -> table;
       table -> table [label="repeat N rounds"];
       table -> output;
   }

