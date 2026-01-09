TrnMenu (Tournament Menu)
=========================

A specialized sizer for tournament mode that features animated hand cursor,
opacity fading, and 2D navigation between buttons positioned at absolute coordinates.

Source: ``src/game/gui/trn_menu.c``

.. seealso:: :doc:`/api/game/gui/trn_menu` for the API reference.


Features
--------

- Absolute positioning of children (like XYSizer)
- 2D spatial navigation (finds nearest button in direction)
- Animated hand cursor that moves between selections
- Opacity fading during transitions
- Submenu support with fade effects
- Button sheet background rendering


Navigation Algorithm
--------------------

Unlike linear menus, TrnMenu uses 2D spatial navigation to find the nearest
selectable button in a given direction.

**Finding Next Button:**

1. Get the position of the currently selected button
2. For each selectable button in the menu:

   - Check if it's in the requested direction
   - Calculate distance to the button
   - Track the closest one found

3. Return the index of the closest button, or -1 if none found

**Direction Checks:**

- ``ACT_LEFT``: Button's x < current x
- ``ACT_RIGHT``: Button's x > current x
- ``ACT_UP``: Button's y < current y
- ``ACT_DOWN``: Button's y > current y


Hand Movement
-------------

When selection changes, the hand cursor animates smoothly from the old position
to the new position.

**Selection Change:**

1. Get the selected component
2. Focus the selected component via :c:func:`component_focus`
3. Save current hand position as start
4. Calculate target position (center of selected button)
5. Initialize movement animation (set ``move = true``, ``moved = 0.0``)

**Movement Animation** (in tick):

1. If ``moved >= 1.0``, movement is complete:

   - Set ``move = false``
   - Set hand position to final pending position
   - If ``return_hand`` mode, select first button

2. Otherwise, interpolate position between start and end based on ``moved`` value
3. Increment ``moved`` by 0.05 each tick


Fade Transitions
----------------

The menu supports opacity fading for smooth transitions.

**Fade States:**

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - State
     - Description
   * - Hidden
     - Opacity = 0, menu not visible
   * - Fading In
     - Opacity increasing from 0 to 1
   * - Visible
     - Opacity = 1, fully visible
   * - Fading Out
     - Opacity decreasing from 1 to 0

**State Transitions:**

- ``Hidden → Fading In``: Menu created or submenu closed
- ``Fading In → Visible``: Opacity reaches 1.0
- ``Visible → Fading Out``: Menu finished or showing submenu
- ``Fading Out → Hidden``: Opacity reaches 0.0


Call Chains
-----------

**Layout** (``trnmenu_layout``):

1. For each child (with index i):

   - If first selectable child: select it, focus it, store selection index
   - Call :c:func:`component_layout` with child's hint values

2. Set hand position to center of selected child

**Tick** (``trnmenu_tick``):

1. If fading: update opacity, check for fade completion

   - Fade in complete: set ``fade = false``
   - Fade out complete: set ``fade = false``; if no submenu, set ``finished = true``

2. If submenu active and not finished: only tick submenu
3. If submenu finished and not fading: start fade in, cleanup submenu
4. If hand.play: tick hand animation
5. If hand.move: interpolate hand position

**Render** (``trnmenu_render``):

1. If not fading and submenu active and not finished: render submenu only
2. If button_sheet exists: draw it
3. For each child: call :c:func:`component_render`
4. If hand animation exists: render hand cursor


Usage Examples
--------------

**Basic Tournament Menu:**

.. code-block:: c

   component *menu = trnmenu_create(&button_bg, 0, 0, false);

   // Add buttons at absolute positions
   component *btn1 = spritebutton_create("FIGHT", &img1, false, fight_cb, NULL);
   component_set_pos_hints(btn1, 50, 100);
   component_set_size_hints(btn1, 80, 30);
   trnmenu_attach(menu, btn1);

   component *btn2 = spritebutton_create("QUIT", &img2, false, quit_cb, NULL);
   component_set_pos_hints(btn2, 150, 100);
   component_set_size_hints(btn2, 80, 30);
   trnmenu_attach(menu, btn2);


**With Hand Animation:**

.. code-block:: c

   component *menu = trnmenu_create(&bg, 0, 0, false);
   // ... add buttons ...

   animation *hand_anim = load_hand_animation();
   trnmenu_bind_hand(menu, hand_anim, game_state);


**With Submenu:**

.. code-block:: c

   void on_done(component *menu, component *submenu) {
       // Handle submenu result
   }

   void on_init(component *menu, component *submenu) {
       // Initialize submenu state
   }

   component *menu = trnmenu_create(&bg, 0, 0, false);
   trnmenu_set_submenu_done_cb(menu, on_done);

   // Later, in button callback:
   component *sub = create_submenu();
   trnmenu_set_submenu_init_cb(sub, on_init);
   trnmenu_set_submenu(menu, sub);


**Return Hand Mode:**

.. code-block:: c

   // Hand returns to first button after each selection
   component *menu = trnmenu_create(&bg, 0, 0, true);
