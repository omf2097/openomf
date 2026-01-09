Text System Overview
====================

The text rendering system handles all text display in OpenOMF, from menu labels
to in-game messages. It provides a flexible, cached layout system with support
for multiple fonts, alignments, shadows, and word wrapping.

Source: ``src/game/gui/text/``

.. seealso:: :doc:`/api/game/gui/text/text` for the API reference.


Architecture
------------

The text system consists of three layers:

.. graphviz::
   :caption: Text System Architecture

   digraph text_arch {
      rankdir=TB
      node [shape=box, style="rounded,filled"]

      subgraph cluster_api {
         label="Public API Layer"
         style=filled
         fillcolor=lightgreen
         text [label="text\n(string + styling)", fillcolor=palegreen]
         doc [label="text_document\n(multi-paragraph)", fillcolor=palegreen]
      }

      subgraph cluster_layout {
         label="Layout Engine Layer"
         style=filled
         fillcolor=lightyellow
         layout [label="text_layout\n(computed positions)", fillcolor=lightyellow]
         items [label="text_layout_item[]\n(glyph references)", fillcolor=lightyellow]
      }

      subgraph cluster_font {
         label="Font System Layer"
         style=filled
         fillcolor=lightpink
         font [label="font\n(dimensions)", fillcolor=mistyrose]
         glyphs [label="glyph surfaces\n(pixel data)", fillcolor=mistyrose]
      }

      text -> layout [label="generates"]
      doc -> layout [label="generates"]
      layout -> items [label="contains"]
      items -> glyphs [label="references"]
      font -> glyphs [label="owns"]
   }

**Public API Layer:**

- :c:struct:`text` - Primary text object for single blocks of text
- :c:struct:`text_document` - Container for multi-paragraph text

**Layout Engine Layer:**

- :c:struct:`text_layout` - Computed glyph positions for a text block
- ``text_layout_item[]`` - Array of positioned glyph references

**Font System Layer:**

- :c:struct:`font` - Font definition with glyph dimensions
- Glyph surfaces - Pre-rendered character images

The text object computes a layout, which contains references to glyph surfaces
owned by the font system.


Text Object
-----------

The :c:struct:`text` object is the primary interface for text rendering. It holds:

- Text content (as a string buffer)
- Styling properties (font, color, alignment, margins, etc.)
- Cached layout (computed glyph positions)

**Key properties:**

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Property
     - Description
   * - ``font``
     - Font size (FONT_BIG, FONT_SMALL, etc.)
   * - ``color``
     - VGA palette index for text
   * - ``shadow_color``
     - VGA palette index for shadow
   * - ``shadow``
     - Shadow direction flags
   * - ``horizontal_align``
     - LEFT, CENTER, or RIGHT
   * - ``vertical_align``
     - TOP, MIDDLE, or BOTTOM
   * - ``margin``
     - Padding inside bounding box
   * - ``line_spacing``
     - Extra space between lines
   * - ``letter_spacing``
     - Extra space between characters
   * - ``word_wrap``
     - Enable automatic line breaking


Layout Caching
--------------

The text system uses lazy layout computation with cache invalidation.

**Cache Behavior:**

1. Layout starts invalid after :c:func:`text_create`
2. Setting layout-affecting properties invalidates the cache
3. :c:func:`text_draw` checks if layout is valid
4. If invalid, :c:func:`text_generate_layout` is called automatically
5. Rendering proceeds using the cached layout

**Properties that invalidate layout:**

- Content changes via :c:func:`text_set_from_c` or :c:func:`text_set_from_str`
- Font changes via :c:func:`text_set_font`
- Bounding box changes via :c:func:`text_set_bounding_box`
- Alignment changes
- Margin changes
- Spacing changes
- Word wrap toggle

**Properties that do NOT invalidate layout:**

- Color changes via :c:func:`text_set_color`
- Shadow color changes via :c:func:`text_set_shadow_color`
- Shadow style changes via :c:func:`text_set_shadow_style`


Rendering Pipeline
------------------

Text rendering uses a two-pass approach:

1. **Layout Check**: If layout is invalid, compute it via :c:func:`text_generate_layout`
2. **Shadow Pass**: Draw glyphs with shadow offset and shadow color
3. **Foreground Pass**: Draw glyphs at normal position with text color


Word Wrapping
-------------

When word wrap is enabled via :c:func:`text_set_word_wrap`, the layout engine
breaks lines at word boundaries.

**Algorithm:**

1. Scan characters from start of line
2. Mark spaces and hyphens as potential break points
3. When accumulated width exceeds bounding box width, break at last marked point
4. Newline characters always cause a line break
5. Continue until end of text


Text Documents
--------------

For multi-paragraph text, use :c:struct:`text_document`:

.. code-block:: c

   text_document *doc = text_document_create();

   // Generate from source with paragraph breaks
   str source;
   str_from_c(&source, "Paragraph 1\\n\\nParagraph 2");
   text_generate_document(doc, &source, FONT_BIG, 200, 300,
                          TEXT_MEDIUM_GREEN, TEXT_DARK_GREEN,
                          TEXT_ALIGN_TOP, TEXT_ALIGN_LEFT,
                          (text_margin){0}, 0, 0, GLYPH_SHADOW_NONE, 0);

   // Render all paragraphs
   text_document_draw(doc, 10, 10);

   text_document_free(&doc);


Usage Examples
--------------

**Basic Text:**

.. code-block:: c

   text *t = text_create_from_c("Hello World");
   text_set_font(t, FONT_BIG);
   text_set_color(t, TEXT_MEDIUM_GREEN);
   text_draw(t, 100, 50);
   text_free(&t);

**Centered Title:**

.. code-block:: c

   text *title = text_create_from_c("MAIN MENU");
   text_set_font(title, FONT_BIG);
   text_set_bounding_box(title, 320, 20);
   text_set_horizontal_align(title, TEXT_ALIGN_CENTER);
   text_set_color(title, TEXT_BRIGHT_GREEN);
   text_draw(title, 0, 10);

**Text with Shadow:**

.. code-block:: c

   text *shadowed = text_create_from_c("Shadowed Text");
   text_set_color(shadowed, TEXT_YELLOW);
   text_set_shadow_color(shadowed, TEXT_SHADOW_YELLOW);
   text_set_shadow_style(shadowed, GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM);
   text_draw(shadowed, 50, 100);

**Word-Wrapped Paragraph:**

.. code-block:: c

   text *para = text_create_from_c(
       "This is a long paragraph that will automatically "
       "wrap to multiple lines based on the bounding box width."
   );
   text_set_font(para, FONT_SMALL);
   text_set_bounding_box(para, 200, TEXT_BBOX_MAX);
   text_set_word_wrap(para, true);
   text_draw(para, 10, 50);

**Get Layout Metrics:**

.. code-block:: c

   text *t = text_create_from_c("Measure me");
   text_set_font(t, FONT_BIG);
   text_generate_layout(t);  // Must call before getting metrics

   uint16_t width = text_get_layout_width(t);
   uint16_t height = text_get_layout_height(t);
   size_t rows = text_get_layout_rows(t);


Integration with GUI Widgets
----------------------------

GUI widgets use text objects internally for rendering:

- **Label** - Holds a text object for display
- **Button** - Uses text for button label
- **TextInput** - Dynamically updates text content
- **TextSelector** - Displays title + current option
- **TextSlider** - Shows title + bar representation

Widgets typically:

1. Create text in widget constructor
2. Configure styling from theme
3. Update content via widget API
4. Call :c:func:`text_draw` in render callback

.. code-block:: c

   // Inside label_render():
   text_set_color(local->text, determine_color(theme));
   text_draw(local->text, c->x, c->y);
