XYSizer
=======

A simple sizer that positions children at absolute coordinates based on their
position and size hints. Unlike Menu, it has no navigation logic.

Source: ``src/game/gui/xysizer.c``

.. seealso:: :doc:`/api/game/gui/xysizer` for the API reference.


Features
--------

- Absolute positioning via x_hint/y_hint
- Fixed sizing via w_hint/h_hint
- No navigation management
- Events passed to all children
- Lightweight implementation


Attach Function
---------------

The :c:func:`xysizer_attach` function sets hints on the child before adding:

.. code-block:: c

   void xysizer_attach(component *c, component *nc, int x, int y, int w, int h) {
       component_set_size_hints(nc, w, h);
       component_set_pos_hints(nc, x, y);
       sizer_attach(c, nc);
   }


Call Chains
-----------

**Layout** (``xysizer_layout``):

For each child:

1. Read position hints (x_hint, y_hint) and size hints (w_hint, h_hint)
2. If size is zero, log a warning (component will be hidden)
3. Call :c:func:`component_layout` with the hint values

**Event** (``xysizer_event``):

For each child:

1. Pass event to child via :c:func:`component_event`
2. If child returns 0 (handled), return 0 immediately
3. If no child handled the event, return 1

**Action** (``xysizer_action``):

For each child:

1. Pass action to child via :c:func:`component_action`
2. If child returns 0 (handled), return 0 immediately
3. If no child handled the action, return 1


Differences from Menu
---------------------

.. list-table::
   :header-rows: 1
   :widths: 25 37 38

   * - Feature
     - Menu
     - XYSizer
   * - Layout
     - Linear (H or V)
     - Absolute (x,y)
   * - Navigation
     - Up/Down or Left/Right
     - None (pass-through)
   * - Selection
     - Tracks selected item
     - None
   * - Background
     - Optional
     - None
   * - Event routing
     - To selected only
     - To all children


Usage Examples
--------------

**Basic Layout:**

.. code-block:: c

   component *sizer = xysizer_create();

   component *title = label_create("TITLE");
   xysizer_attach(sizer, title, 100, 10, 120, 20);

   component *btn = button_create("OK", NULL, false, true, cb, NULL);
   xysizer_attach(sizer, btn, 100, 50, 60, 15);

   component *img = spriteimage_create(&sprite);
   xysizer_attach(sizer, img, 10, 10, sprite.w, sprite.h);


**Mixed Content:**

.. code-block:: c

   component *layout = xysizer_create();

   // Image in top-left
   xysizer_attach(layout, spriteimage_create(&logo), 0, 0, 64, 64);

   // Menu on the right
   component *menu = menu_create();
   menu_attach(menu, button_create("Item 1", NULL, false, false, cb, NULL));
   menu_attach(menu, button_create("Item 2", NULL, false, false, cb, NULL));
   xysizer_attach(layout, menu, 80, 0, 200, 100);


**HUD Layout:**

.. code-block:: c

   component *hud = xysizer_create();

   // Health bar (top-left)
   component *health = progressbar_create(PROGRESSBAR_THEME_HEALTH,
                                          PROGRESSBAR_LEFT, 100);
   xysizer_attach(hud, health, 10, 10, 100, 8);

   // Endurance bar (top-left, below health)
   component *endurance = progressbar_create(PROGRESSBAR_THEME_ENDURANCE,
                                             PROGRESSBAR_LEFT, 100);
   xysizer_attach(hud, endurance, 10, 22, 100, 8);

   // Score (top-right)
   component *score = label_create("0000000");
   xysizer_attach(hud, score, 250, 10, 60, 12);


Important Notes
---------------

**Size Hints Required:**
   Components must have size hints set, otherwise they will be positioned
   at (0,0) with size (0,0) and a warning will be logged.

**No Selection Management:**
   XYSizer doesn't track which child is selected. If you need selection
   behavior, use Menu or TrnMenu instead.

**Event Broadcast:**
   Events are sent to ALL children until one handles them. This means
   multiple interactive widgets could theoretically all respond to the
   same event (though typically only focused ones respond).
