TextInput
=========

A text entry widget that accepts keyboard input with cursor navigation,
character scrolling, and optional filtering.

Source: ``src/game/gui/textinput.c``

.. seealso:: :doc:`/api/game/gui/textinput` for the API reference.


Features
--------

- Character-by-character input with cursor
- Left/Right cursor movement
- Up/Down character scrolling (0-9, space, A-Z)
- Backspace and Delete key support
- Ctrl+V paste from clipboard
- Optional background box
- Character filter callback
- Done callback when confirmed
- Configurable font and alignment


Input Handling
--------------

**Action Events:**

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Action
     - Behavior
   * - ``ACT_LEFT``
     - Move cursor left
   * - ``ACT_RIGHT``
     - Move cursor right (extends text if at end)
   * - ``ACT_UP``
     - Scroll character up at cursor (A->B, 9->A, space->0)
   * - ``ACT_DOWN``
     - Scroll character down at cursor (B->A, A->9, 0->space)
   * - ``ACT_PUNCH``
     - Call done callback

**SDL Events:**

- **Text Input**: Appends typed character (filtered)
- **Backspace**: Delete character before cursor
- **Delete**: Delete character at cursor
- **Ctrl+V**: Paste from clipboard


Character Scrolling Order
-------------------------

The character set is reordered for convenience: ``0-9``, ``space``, ``A-Z``

Scrolling wraps: ``0`` → ... → ``9`` → ``space`` → ``A`` → ... → ``Z`` → ``0``


Call Chains
-----------

**Render** (``textinput_render``):

1. If background enabled, draw background box
2. Determine state and set cursor/color:

   .. list-table::
      :header-rows: 1
      :widths: 25 35 40

      * - State
        - Cursor
        - Color
      * - Selected
        - Visible
        - ``theme->text.active_color``
      * - Disabled
        - Hidden
        - ``theme->text.disabled_color``
      * - Inactive
        - Hidden
        - ``theme->text.inactive_color``

3. Draw text via :c:func:`text_draw`


**Focus Change** (internal callback):

1. Check if state actually changed
2. If now focused:

   - Insert cursor character into display text at cursor position

3. If unfocused:

   - Remove cursor character
   - Restore original text


Usage Examples
--------------

**Basic Text Input:**

.. code-block:: c

   void on_done(component *c, void *userdata) {
       const char *name = textinput_value(c);
       printf("Entered: %s\n", name);
   }

   component *input = textinput_create(16, "Enter your name", "");
   textinput_set_done_cb(input, on_done, NULL);

**Pre-filled Input:**

.. code-block:: c

   component *input = textinput_create(8, NULL, "DEFAULT");

**Filtered Input (Numbers Only):**

.. code-block:: c

   bool numbers_only(char c) {
       return c >= '0' && c <= '9';
   }

   component *input = textinput_create(4, "Enter number", "");
   textinput_set_filter_cb(input, numbers_only);

**Without Background:**

.. code-block:: c

   component *input = textinput_create(20, NULL, "");
   textinput_enable_background(input, false);

**Getting/Setting Value:**

.. code-block:: c

   // Get current value (strips trailing spaces)
   const char *value = textinput_value(input);

   // Set new value
   textinput_set_text(input, "New Value");

   // Clear
   textinput_clear(input);


Notes
-----

- The ``CURSOR_CHAR`` is a special character that displays as a block
- Invalid characters (``@`` and ``~``) are automatically filtered
- Text is stripped of trailing spaces when retrieved via :c:func:`textinput_value`
- Background is enabled by default
