Rendering Pipeline
==================

OpenOMF renders all graphics using **paletted indexing** through an OpenGL 3.3 pipeline. Every
sprite is an 8-bit indexed surface -- its pixels are palette indices, not colors. The renderer
collects all draw calls during a frame, batches them by blend mode, and resolves the final RGB
output through three passes: indexed rendering, palette resolve, and screen scaling.

For palette layout, color zones, and palette transforms, see :doc:`palette`.

Architecture
------------

The renderer uses a plugin-style interface (``renderer.h``) with function pointers for
initialization, drawing, and frame management. The primary backend is **OpenGL3**
(``gl3_renderer.c``); a no-op **Null** backend exists as a fallback. Game code uses the
``video.h`` public API, which delegates to whichever backend is active.

The game's native resolution is **320x200**. Internal framebuffers are scaled by a configurable
``fb_scale`` multiplier. The final output is scaled to the window with aspect ratio correction
(4:3 or stretch) and any active screen shake offset.

Texture Atlas
-------------

All sprites are packed into a single 2048x2048 texture. Each surface gets a unique ID at
creation; on first draw it is bin-packed into the atlas and cached in a hash map (see
:doc:`sprite_packer` for the packing algorithm). The atlas is cleared on scene changes.

Draw Calls and Batching
-----------------------

Game code submits sprites through the ``video_draw*`` family. Each call looks up the sprite in
the atlas and appends a quad to the vertex buffer. No GL draw calls happen yet -- everything is
deferred to ``render_finish()``.

The vertex buffer holds up to **2048 quads** per frame. Each quad carries per-vertex attributes
for position, atlas coordinates, transparency index, remap table/rounds, palette offset/limit,
decimation value, and effect flags. Sprites are tagged with a blend mode and consecutive
same-mode sprites are batched into single ``glMultiDrawArrays`` calls.

Three-Pass Pipeline
-------------------

Pass 1: Indexed Rendering (palette.frag)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All sprites are drawn into a framebuffer with four 16-bit channels (RGBA16). This is not a
regular color buffer -- each channel encodes a different piece of information for the resolve
pass.

Effects like tints, remaps, and shadows are drawn as separate quads layered on top of the base
sprite. Without protection, these overlay draws would overwrite the base palette index already
stored in the R channel. ``glColorMask`` prevents this: each blend mode masks off the channels
it does not own, so an overlay draw can only touch its designated channel(s) while leaving the
rest intact.

======================  ====  ====  ====  ====  ======================================
Mode                    R     G     B     A     What it writes
======================  ====  ====  ====  ====  ======================================
``MODE_SET``            yes   yes   yes   yes   Base sprite index
``MODE_DARK_TINT``      --    yes   yes   yes   Tint overlay
``MODE_REMAP``          --    yes   --    --    Remap info overlay
``MODE_SPRITE_SHADOW``  --    yes   --    --    Shadow coverage (max-blended)
``MODE_ADD``            --    --    --    yes   Additive index (credits)
======================  ====  ====  ====  ====  ======================================

The fragment shader processes each sprite through: pixel decimation (dithered transparency),
shadow mask sampling, transparency discard, palette offset/limit adjustment, remap table
lookup, and channel encoding.

Each sprite carries a ``palette_offset`` and ``palette_limit``. Any index within the limit is
shifted by the offset and clamped. This is how both players share the same HAR sprite data --
player 1 uses offset 0, player 2 uses offset 48 to shift indices into its color zone. See
:doc:`palette` for the full color zone layout.

Pass 2: Palette Resolve (rgba.frag)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A fullscreen quad reads the paletted framebuffer and reassembles each pixel from the encoded
channels. The R channel provides the base palette index, G encodes a remap table selector and
round count, B carries a dark tint index, and A holds any additive contribution.

The shader first combines the base index with any additive offset. It then checks for dark
tint: if a tint index is present, it replaces or blends with the base index using a
brightness lookup through a remap table. Finally, the remap rounds are applied -- the index
is fed through the selected remap table the requested number of times (see :doc:`palette` for
how remap tables work). The resulting index is looked up in the palette texture to produce the
final RGB color.

Pass 3: Screen Scaling
~~~~~~~~~~~~~~~~~~~~~~~

The resolved RGBA framebuffer is still at internal resolution (320x200 times ``fb_scale``).
A final fullscreen quad scales it to the window. The viewport is letterboxed to 4:3 aspect
ratio (or stretched, depending on settings), and any active screen shake is applied as a
viewport offset. The user can choose between three scaling filters:

==============  ===========================================================================
Mode            Shader
==============  ===========================================================================
0 (None)        ``scalers/none.frag`` -- nearest-neighbor passthrough
1 (Bilinear)    ``scalers/bilinear.frag`` -- bilinear interpolation of 4 neighboring texels
2 (CRT)         ``scalers/crt.frag`` -- scanline darkening and color bleed simulation
==============  ===========================================================================

The CRT filter simulates a cathode-ray display by darkening alternating scanlines and blending
each pixel with its left and top neighbors to approximate color bleed.

Sprite Flags
------------

Per-sprite bit flags control which rendering path a sprite takes:

======================  =========================================================
Flag                    Effect
======================  =========================================================
``SPRITE_REMAP``        Look up the index in a remap table during pass 1
``SPRITE_SHADOW``       Render as a shadow mask (4-sample coverage at 1/4 height)
``SPRITE_INDEX_ADD``    Write to the additive channel (credits)
``SPRITE_HAR_QUIRKS``   Skip remap for indices > 0x30 (Electra/Pyros)
``SPRITE_DARK_TINT``    Use the multi-channel dark tint / stasis path
======================  =========================================================

``object_render()`` translates game-level effects (glow, trail, dark tint, stasis, additive)
into combinations of these flags and remap parameters.

Offscreen Rendering
-------------------

The renderer can also draw to a CPU-accessible ``screen_surface`` instead of the screen. This
runs pass 1 only and reads back the R channel. The result uses 16-bit palette indices
(supporting the full 1024-color palette) and can be converted to a standard 8-bit surface.
