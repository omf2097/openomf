SpriteButton
============

A button widget that displays a sprite image with optional text overlay.
Features visual feedback on activation and focus callbacks.

Source: ``src/game/gui/spritebutton.c``

.. seealso:: :doc:`/api/game/gui/spritebutton` for the API reference.


Features
--------

- Sprite image display (externally owned)
- Optional text overlay with configurable positioning
- Visual feedback on click (10 tick highlight)
- Focus callback for additional behaviors
- Custom tick callback
- Configurable text color, font, alignment, and margins
- Disabled state shows grayed-out sprite


Display States
--------------

The sprite button has three display states:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - State
     - Behavior
   * - Disabled
     - Shows grayed-out sprite
   * - Active (``active_ticks > 0``)
     - Shows normal sprite
   * - Inactive (``active_ticks == 0``)
     - Shows nothing (sprite hidden)

The "always display" mode sets ``active_ticks`` to -1, preventing it from
reaching 0 and hiding the sprite.


Call Chains
-----------

**Render** (``spritebutton_render``):

1. Check if disabled via :c:func:`component_is_disabled`:

   - If disabled, draw sprite with gray tint
   - Skip to text check

2. If not disabled, check if active (``active_ticks != 0``):

   - If active, draw sprite normally

3. Check if text is set:

   - Select text color based on state (selected/disabled/inactive)
   - Draw text at configured position with alignment


**Tick** (``spritebutton_tick``):

1. If ``active_ticks > 0``, decrement active ticks
2. If tick callback is set, call tick callback


**Action** (``spritebutton_action``):

1. Check if disabled via :c:func:`component_is_disabled`:

   - If disabled, return 1 (not handled)

2. Check if action is ``ACT_KICK`` or ``ACT_PUNCH``:

   - If ``active_ticks >= 0``, set ``active_ticks = 10`` (visual feedback)
   - If click callback is set, call click callback
   - Return 0 (handled)

3. Otherwise, return 1 (not handled)


Usage Examples
--------------

**Basic Sprite Button:**

.. code-block:: c

   void on_click(component *c, void *userdata) {
       printf("Button clicked!\n");
   }

   // surface is owned elsewhere, must outlive the button
   component *btn = spritebutton_create(NULL, &my_surface, false, on_click, NULL);

**With Text Overlay:**

.. code-block:: c

   component *btn = spritebutton_create("ATTACK", &attack_img, false, attack_cb, NULL);
   spritebutton_set_horizontal_align(btn, TEXT_ALIGN_CENTER);
   spritebutton_set_vertical_align(btn, TEXT_ALIGN_BOTTOM);
   spritebutton_set_text_margin(btn, (text_margin){0, 0, 4, 0});

**Always Visible:**

.. code-block:: c

   component *btn = spritebutton_create(NULL, &icon_img, false, cb, NULL);
   spritebutton_set_always_display(btn);  // Never hide sprite

**With Focus Callback:**

.. code-block:: c

   void on_focus(component *c, bool focused, void *userdata) {
       if(focused) {
           play_hover_sound();
       }
   }

   component *btn = spritebutton_create(NULL, &img, false, click_cb, NULL);
   spritebutton_set_focus_cb(btn, on_focus);

**Changing Image:**

.. code-block:: c

   spritebutton_set_img(btn, &new_surface);

**Auto-free Userdata:**

.. code-block:: c

   my_data *data = omf_malloc(sizeof(my_data));
   component *btn = spritebutton_create(NULL, &img, false, cb, data);
   spritebutton_set_free_userdata(btn, true);  // Free data when button freed
