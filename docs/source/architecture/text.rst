Text Rendering Architecture
===========================

OpenOMF uses a layered text rendering system that handles font loading, text layout computation,
and styled text rendering. The system is designed around heavy caching and fast rendering.

Architecture Layers
-------------------

The text rendering system is organized in three layers:

- **Font Manager** (lowest layer) handles loading font files and providing glyph surfaces. All fonts
  are loaded at startup, with glyphs pre-decoded to surfaces for fast access.
- **Text Layout** (middle layer) computes glyph positions within a bounding box. Handles line
  breaking, word wrapping and alignment.
- **Text Object** (highest layer) handles text content with styling options (colors, shadows,
  alignment). Manages layout caching and provides the rendering API.

Font Management
---------------

Four fonts are loaded at startup:

============  =============  ================
Font          Height         Source File
============  =============  ================
FONT_BIG      8px            GRAPHCHR.DAT
FONT_SMALL    6px            CHARSMAL.DAT
FONT_NET1     varies         NETFONT1.PCX
FONT_NET2     varies         NETFONT2.PCX
============  =============  ================

Each font stores a vector of 224 pre-decoded glyph surfaces. Glyphs are retrieved via
``font_get_surface(font, character)`` in constant time.

Text Layout Pipeline
--------------------

Layout computation transforms text content into positioned glyphs:

1. **Line Breaking**: Text is split into lines respecting bounding box width.
   Supports word wrapping (break at spaces/hyphens) or hard wrapping.

2. **Row Metrics**: Width and height calculated for each row, accounting for
   letter spacing and glyph dimensions.

3. **Alignment**: Horizontal (left/center/right) and vertical (top/middle/bottom)
   offsets computed.

4. **Layout Items**: Final output is a vector of positioned glyphs, each with
   (x, y) coordinates and a pointer to the glyph surface.

Cache Invalidation
------------------

Layout computation is expensive, so results are cached. The cache uses invalidation flags:

==================  =======================  ===================================
Flag                Invalidates              Triggered By
==================  =======================  ===================================
INVALIDATE_LAYOUT   Layout (positions)       Font, bounding box, alignment,
                                             margins, spacing, word wrap changes
INVALIDATE_STYLE    Style (colors/shadows)   Color or shadow changes
INVALIDATE_ALL      Everything               Text content changes
==================  =======================  ===================================

Color changes only require re-rendering, not re-layout. This optimization avoids expensive
line-breaking calculations when only styling changes.

Text Styling
------------

Text objects support styling options:

**Colors** use VGA palette indices (0x00-0xFF):

- Main text color
- Shadow color (rendered behind text)

**Shadow Styles** are bitflags controlling shadow direction:

===================  ================
Flag                 Effect
===================  ================
GLYPH_SHADOW_TOP     Shadow above
GLYPH_SHADOW_BOTTOM  Shadow below
GLYPH_SHADOW_LEFT    Shadow left
GLYPH_SHADOW_RIGHT   Shadow right
GLYPH_SHADOW_ALL     All directions
===================  ================

**Alignment**:

- Vertical: TOP, MIDDLE, BOTTOM
- Horizontal: LEFT, CENTER, RIGHT

**Spacing**:

- Extra pixels between lines
- Extra pixels between glyphs
- Margin around each glyph

Rendering Flow
--------------

.. graphviz::

   digraph render_flow {
       rankdir=TB
       node [shape=box, fontname="monospace"]

       draw [label="text_draw(text, x, y)"]
       cache [label="Cache valid?", shape=diamond]
       generate [label="text_generate_layout()"]
       shadows [label="Draw shadows\n(first pass)"]
       foreground [label="Draw glyphs\n(second pass)"]
       screen [label="Screen", shape=ellipse]

       draw -> cache
       cache -> generate [label="no"]
       cache -> shadows [label="yes"]
       generate -> shadows
       shadows -> foreground -> screen
   }

Rendering is a two-pass process:

1. **Shadow pass**: Draw shadow pixels at offset positions using ``shadow_color``
2. **Foreground pass**: Draw glyph pixels at exact positions using ``text_color``

The two-pass approach prevents shadow overlap issues.

Text Documents
--------------

For complex formatted text, the system provides ``text_document`` which holds multiple text
objects with different styles. Documents support markup commands embedded in text:

=================  =============================
Command            Effect
=================  =============================
{CENTER ON/OFF}    Toggle text centering
{SIZE 8/6}         Switch font size
{SHADOWS ON/OFF}   Toggle shadow rendering
{COLOR:YELLOW}     Change to predefined color
{COLOR <n>}        Set specific palette index
{WIDTH <n>}        Set bounding box width
{VMOVE <n>}        Vertical offset
{SPACING <n>}      Set line spacing
=================  =============================

Documents are generated by parsing markup, splitting text into chunks, and creating separate
text objects for each chunk with accumulated styling.
