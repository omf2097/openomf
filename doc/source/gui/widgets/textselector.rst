TextSelector
============

A dropdown-style selector widget that cycles through a list of text options.

Source: ``src/game/gui/textselector.c``

.. seealso:: :doc:`/api/game/gui/textselector` for the API reference.


Features
--------

- Displays label followed by current option
- Cycles through options with left/right actions
- Optional title prefix
- Toggle callback on selection change
- Bindable position to external variable
- Configurable font and alignment


Display Format
--------------

The selector displays text in this format:

- With title: ``"TITLE OPTION"``
- Without title: ``"OPTION"``
- No options: ``"TITLE -"``


Input Handling
--------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Action
     - Behavior
   * - ``ACT_KICK``
     - Next option (wraps)
   * - ``ACT_PUNCH``
     - Next option (wraps)
   * - ``ACT_RIGHT``
     - Next option (wraps)
   * - ``ACT_LEFT``
     - Previous option (wraps)

Audio feedback plays when the selection changes (sound 20 with panning).


Call Chains
-----------

**Action** (``textselector_action``):

1. If options count is 0 or 1, return 0 (nothing to select)
2. Check action type:

   .. list-table::
      :header-rows: 1
      :widths: 30 70

      * - Action
        - Operation
      * - ``ACT_KICK``, ``ACT_PUNCH``, ``ACT_RIGHT``
        - Increment position (wrap at max)
      * - ``ACT_LEFT``
        - Decrement position (wrap at 0)

3. If position changed:

   - Refresh display text
   - Call toggle callback if set
   - Play selection sound via :c:func:`audio_play_sound`
   - Return 0 (handled)

4. If position unchanged, return 1 (not handled)


Binding
-------

Position can be bound to an external variable for automatic synchronization:

.. code-block:: c

   int difficulty = 1;  // External variable

   component *sel = textselector_create_bind(
       "DIFFICULTY",
       "Select game difficulty",
       on_difficulty_change,
       NULL,
       &difficulty  // Bound to this
   );

   textselector_add_option(sel, "EASY");
   textselector_add_option(sel, "NORMAL");
   textselector_add_option(sel, "HARD");

   // difficulty now reflects current selection


Usage Examples
--------------

**Simple Selector:**

.. code-block:: c

   component *sel = textselector_create("MODE", NULL, NULL, NULL);
   textselector_add_option(sel, "EASY");
   textselector_add_option(sel, "NORMAL");
   textselector_add_option(sel, "HARD");

**With Callback:**

.. code-block:: c

   void on_change(component *c, void *userdata, int pos) {
       printf("Selected option %d\n", pos);
   }

   component *sel = textselector_create("SOUND", "Toggle sound", on_change, NULL);
   textselector_add_option(sel, "ON");
   textselector_add_option(sel, "OFF");

**Pre-populated with Binding:**

.. code-block:: c

   static const char *options[] = {"LOW", "MEDIUM", "HIGH"};
   int setting = 1;

   component *sel = textselector_create_bind_opts(
       "QUALITY",
       NULL,
       callback,
       NULL,
       &setting,
       options,
       3
   );

**Dynamic Options:**

.. code-block:: c

   // Clear and rebuild options
   textselector_clear_options(sel);
   textselector_add_option(sel, "NEW OPTION 1");
   textselector_add_option(sel, "NEW OPTION 2");
   textselector_set_pos(sel, 0);
