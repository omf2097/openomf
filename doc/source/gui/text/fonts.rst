Font System
===========

OpenOMF uses a VGA-style font system with pre-rendered glyph surfaces loaded
from game resource files.

Source: ``src/resources/fonts.h``, ``src/resources/fonts.c``

.. seealso::

   See ``src/resources/fonts.h`` for the API reference.


Available Fonts
---------------

Four font sizes are available:

.. list-table::
   :header-rows: 1
   :widths: 20 15 30 35

   * - Font
     - Height
     - Source File
     - Usage
   * - ``FONT_BIG``
     - 8px
     - ``GRAPHCHR.DAT``
     - Titles, menu items
   * - ``FONT_SMALL``
     - 6px
     - ``CHRSMAL.DAT``
     - Help text, descriptions
   * - ``FONT_NET1``
     - varies
     - ``NETFONT1.PCX``
     - Network interface
   * - ``FONT_NET2``
     - varies
     - ``NETFONT2.PCX``
     - Network interface


Font Loading
------------

Fonts are loaded at startup via :c:func:`fonts_init`. The loading sequence is:

1. Load ``GRAPHCHR.DAT`` for ``FONT_BIG``
2. Load ``CHRSMAL.DAT`` for ``FONT_SMALL``
3. Load ``NETFONT1.PCX`` for ``FONT_NET1``
4. Load ``NETFONT2.PCX`` for ``FONT_NET2``

All fonts must load successfully for the game to start.


Glyph Storage
-------------

Each :c:struct:`font` stores pre-rendered glyph surfaces in a vector:

**Font Structure:**

- ``size`` - Font identifier (FONT_BIG, FONT_SMALL, etc.)
- ``w``, ``h`` - Glyph dimensions in pixels
- ``surfaces`` - Vector of surface pointers, one per character

**Character Mapping:**

Characters are indexed by ASCII value offset from space (0x20):

.. code-block:: c

   // Character 'A' (0x41) maps to index:
   // 0x41 - 0x20 = 0x21 = 33

The vector contains approximately 233 glyph surfaces covering printable ASCII
and extended characters.


Retrieving Glyphs
-----------------

Use :c:func:`font_get_surface` to get a glyph:

.. code-block:: c

   const font *f = fonts_get_font(FONT_BIG);
   const surface *glyph = font_get_surface(f, 'A');

   if(glyph) {
       video_draw(glyph, x, y);
   }


Color System
------------

Fonts use VGA palette-based coloring. The text system applies color by
rendering with a palette offset:

.. code-block:: c

   // Color rendering uses:
   // palette_offset = (int)color - 1

Common color constants:

.. list-table::
   :header-rows: 1
   :widths: 35 15 50

   * - Constant
     - Value
     - Description
   * - ``TEXT_DARK_GREEN``
     - 0xFE
     - Dark green text
   * - ``TEXT_MEDIUM_GREEN``
     - 0xFE
     - Medium green (same as dark)
   * - ``TEXT_BRIGHT_GREEN``
     - 0xFD
     - Bright green highlight
   * - ``TEXT_BLINKY_GREEN``
     - 0xFF
     - Animated blinking green
   * - ``TEXT_TRN_BLUE``
     - 0xAB
     - Tournament mode blue
   * - ``TEXT_YELLOW``
     - 0xF8
     - Yellow text
   * - ``TEXT_SHADOW_YELLOW``
     - 0xC0
     - Yellow shadow


Usage Examples
--------------

**Initialize Fonts:**

.. code-block:: c

   if(!fonts_init()) {
       // Handle font loading failure
       return false;
   }

**Get Font for Text:**

.. code-block:: c

   const font *big = fonts_get_font(FONT_BIG);
   const font *small = fonts_get_font(FONT_SMALL);

**Clean Up:**

.. code-block:: c

   fonts_close();  // Call at shutdown


Theme Integration
-----------------

The GUI theme specifies default fonts for widgets:

.. code-block:: c

   typedef struct gui_text_theme {
       font_size font;            // Default font
       vga_index primary_color;   // Main text color
       vga_index secondary_color; // Alternative text color
       vga_index active_color;    // Selected state
       vga_index inactive_color;  // Unselected state
       vga_index disabled_color;  // Disabled state
       vga_index shadow_color;    // Shadow color
   } gui_text_theme;

Widgets read the theme's font setting unless overridden:

.. code-block:: c

   // In widget init:
   text_set_font(t, theme->text.font);
