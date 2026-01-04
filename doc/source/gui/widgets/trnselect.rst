TrnSelect
=========

A specialized widget for selecting tournaments, displaying tournament logos
and descriptions loaded from TRN resource files.

Source: ``src/game/gui/trnselect.c``

.. seealso:: :doc:`/api/game/gui/trnselect` for the API reference.


Features
--------

- Loads tournament data from TRN files
- Displays tournament logo
- Shows tournament description as a child label
- Next/previous navigation
- Automatic palette management


State Support
-------------

TrnSelect is display-only:

- ``supports_disable`` = false
- ``supports_select`` = false
- ``supports_focus`` = false


Call Chains
-----------

**Navigation** (:c:func:`trnselect_next` / :c:func:`trnselect_prev`):

1. Increment/decrement selection index
2. Check bounds:

   - If at max, wrap to first
   - If below zero, wrap to last

3. Get selected tournament from list
4. Get logo sprite from tournament
5. Set tournament palette via :c:func:`vga_state_set_base_palette_from_range`
6. Load description label with tournament text
7. Free old display sprite
8. Create new display sprite from logo


**Render** (``trnselect_render``):

1. Draw logo sprite at component position
2. If description label exists:

   - Render description label via :c:func:`component_render`


**Free** (``trnselect_free``):

1. Restore saved palette via :c:func:`vga_state_pop_palette`
2. If image exists, free sprite memory
3. Free tournament list
4. If description label exists, free label component


Usage Examples
--------------

**Tournament Selection Screen:**

.. code-block:: c

   component *selector = trnselect_create();

   // In action handler
   void on_action(int action) {
       if(action == ACT_LEFT) {
           trnselect_prev(selector);
       } else if(action == ACT_RIGHT) {
           trnselect_next(selector);
       } else if(action == ACT_PUNCH) {
           sd_tournament_file *trn = trnselect_selected(selector);
           start_tournament(trn);
       }
   }

**Getting Selected Tournament:**

.. code-block:: c

   sd_tournament_file *tournament = trnselect_selected(selector);
   printf("Selected: %s\n", tournament->locales[0]->title);


Palette Management
------------------

The widget manages VGA palette state:

1. On init: Saves current palette with :c:func:`vga_state_push_palette`
2. On navigation: Updates palette range 128-168 with tournament colors
3. On free: Restores original palette with :c:func:`vga_state_pop_palette`

This ensures tournament-specific colors display correctly without
corrupting the rest of the game's palette.


Tournament Data
---------------

Each tournament (:c:struct:`sd_tournament_file`) contains:

- Palette for colors
- Locales (language-specific data)

  - Logo sprite
  - Description text
  - Description positioning (width, center, vmove)
  - Color settings
