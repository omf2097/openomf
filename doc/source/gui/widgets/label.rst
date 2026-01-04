Label
=====

A non-interactive widget for displaying text. Labels support various formatting
options including alignment, color, font, shadow, and margins.

Source: ``src/game/gui/label.c``

.. seealso:: :doc:`/api/game/gui/label` for the API reference.


Features
--------

- Multiple text alignments (horizontal and vertical)
- Primary and secondary color themes
- Custom color override
- Font selection
- Text shadow support
- Configurable margins and letter spacing
- Word wrapping support


State Support
-------------

Labels are display-only widgets:

- ``supports_disable`` = true (affects nothing visually by default)
- ``supports_select`` = false
- ``supports_focus`` = false


Call Chains
-----------

**Initialization** (:c:func:`label_create`):

1. Apply font (override if set, otherwise from theme)
2. Apply letter spacing
3. Set horizontal alignment
4. Apply shadow style
5. Apply shadow color
6. Apply text margin
7. Set bounding box
8. If ``w_hint < 0``:

   - Generate text layout
   - Set width hint from text width + padding

9. If ``h_hint < 0``:

   - Generate text layout
   - Set height hint from text height + padding


**Render** (``label_render``):

1. Determine text color:

   .. list-table::
      :header-rows: 1
      :widths: 30 70

      * - Condition
        - Color Used
      * - ``override_color >= 0``
        - Use override color directly
      * - Secondary theme (theme 1)
        - ``theme->text.secondary_color``
      * - Primary theme (theme 0)
        - ``theme->text.primary_color``

2. Apply selected color via :c:func:`text_set_color`
3. Draw text via :c:func:`text_draw`


Usage Examples
--------------

**Simple Label:**

.. code-block:: c

   component *label = label_create("Hello World");

**Title Label:**

.. code-block:: c

   // Centered with secondary color
   component *title = label_create_title("MAIN MENU");

**Multiline Label with Width:**

.. code-block:: c

   component *desc = label_create_with_width(
       "This is a long description that will wrap at 200 pixels width.",
       200
   );

**Custom Color:**

.. code-block:: c

   component *label = label_create("Warning!");
   label_set_text_color(label, 0xE5);  // Red

**Aligned Label:**

.. code-block:: c

   component *label = label_create("Right Aligned");
   label_set_text_horizontal_align(label, TEXT_ALIGN_RIGHT);
   label_set_text_vertical_align(label, TEXT_ALIGN_MIDDLE);

**Label with Shadow:**

.. code-block:: c

   component *label = label_create("Shadowed Text");
   label_set_text_shadow(label, GLYPH_SHADOW_RIGHT, 0xA0);

**Label with Margins:**

.. code-block:: c

   component *label = label_create("Padded Text");
   label_set_margin(label, (text_margin){4, 4, 4, 4});


Color Themes
------------

Labels support two color themes from the :c:struct:`gui_theme`:

- **Theme 0 (primary)**: Uses ``theme->text.primary_color``
- **Theme 1 (secondary)**: Uses ``theme->text.secondary_color``

.. code-block:: c

   label_set_color_theme(label, 1);  // Use secondary color
