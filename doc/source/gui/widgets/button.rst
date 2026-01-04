Button
======

A clickable button widget that displays text and triggers a callback when activated.

Source: ``src/game/gui/button.c``

.. seealso:: :doc:`/api/game/gui/button` for the API reference.


Features
--------

- Text label with configurable shadow
- Optional decorative border
- State-based color changes (active/inactive/disabled)
- Click callback with userdata
- Help text support


Call Chains
-----------

**Initialization** (:c:func:`button_create`):

1. Apply theme font to text renderer
2. Apply theme color
3. Apply shadow style
4. If width hint is unset:

   - Generate text layout
   - Set width hint from text width + border padding

5. If height hint is unset:

   - Generate text layout
   - Set height hint from text height + border padding


**Layout** (``button_layout``):

1. Center text horizontally within component bounds
2. If border is enabled:

   - Set text bounding box with border inset
   - Create border surface via :c:func:`surface_create`

3. Otherwise, set text bounding box to full component bounds


**Render** (``button_render``):

1. Get widget data via :c:func:`widget_get_obj`
2. Get theme via :c:func:`component_get_theme`
3. Select text color based on state:

   .. list-table::
      :header-rows: 1
      :widths: 30 70

      * - State
        - Color Used
      * - Selected
        - ``theme->text.active_color``
      * - Disabled
        - ``theme->text.disabled_color``
      * - Inactive
        - ``theme->text.inactive_color``

4. If border is enabled:

   - Draw border surface
   - Draw text with border offset

5. Otherwise, draw text at component position


**Action** (``button_action``):

1. Check if action is ``ACT_KICK`` or ``ACT_PUNCH``
2. If yes:

   - If callback is set, invoke click callback
   - Play click sound via :c:func:`audio_play_sound`
   - Return 0 (handled)

3. Otherwise, return 1 (not handled)


Usage Examples
--------------

**Simple Button:**

.. code-block:: c

   void on_click(component *c, void *userdata) {
       printf("Button clicked!\n");
   }

   component *btn = button_create("Click Me", NULL, false, false, on_click, NULL);

**Button with Help Text:**

.. code-block:: c

   component *btn = button_create(
       "Start Game",
       "Begin a new game session",
       false,  // not disabled
       false,  // no border
       start_game_cb,
       NULL
   );

**Bordered Button:**

.. code-block:: c

   component *btn = button_create("OK", NULL, false, true, ok_cb, NULL);

**Button with Shadow:**

.. code-block:: c

   component *btn = button_create("Shadowed", NULL, false, false, cb, NULL);
   button_set_text_shadow(btn, GLYPH_SHADOW_RIGHT, 0xA0);

**Disabled Button:**

.. code-block:: c

   component *btn = button_create("Unavailable", NULL, true, false, NULL, NULL);

**Changing Text:**

.. code-block:: c

   button_set_text(btn, "New Label");
