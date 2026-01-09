Filler
======

An invisible widget used as a layout placeholder or spacer within sizers.

Source: ``src/game/gui/filler.c``

.. seealso:: :doc:`/api/game/gui/filler` for the API reference.


Features
--------

- No visual representation
- No user interaction
- Takes up space in layouts
- Useful for flexible spacing


Implementation
--------------

The filler is the simplest possible widget:

.. code-block:: c

   component *filler_create(void) {
       component *c = widget_create();
       component_disable(c, true);
       component_set_supports(c, true, false, false);
       return c;
   }

It has:

- No render callback (nothing to draw)
- No action callback (nothing to interact with)
- No init/layout callbacks (no special behavior)
- Only basic widget lifecycle


State Support
-------------

- ``supports_disable`` = true (always disabled)
- ``supports_select`` = false
- ``supports_focus`` = false

The filler is created in disabled state by default.


Usage Examples
--------------

**Vertical Spacing:**

.. code-block:: c

   component *menu = menu_create();
   menu_attach(menu, label_create("Title"));
   menu_attach(menu, filler_create());  // Spacer
   menu_attach(menu, button_create("Start", NULL, false, false, cb, NULL));
   menu_attach(menu, button_create("Quit", NULL, false, false, cb, NULL));

**Flexible Space:**

In menus without fixed size hints, the filler expands to fill available space:

.. code-block:: c

   // Header - expands - Footer layout
   component *menu = menu_create();
   menu_attach(menu, label_create("HEADER"));
   menu_attach(menu, filler_create());  // Expands to fill space
   menu_attach(menu, label_create("FOOTER"));

**Fixed-Size Spacer:**

.. code-block:: c

   component *spacer = filler_create();
   component_set_size_hints(spacer, -1, 20);  // 20px tall
   menu_attach(menu, spacer);


Behavior in Different Sizers
----------------------------

**In Menu (vertical):**
   Takes up vertical space. Without hints, gets equal share of remaining space.

**In Menu (horizontal):**
   Takes up horizontal space. Without hints, gets equal share of remaining space.

**In XYSizer:**
   Must have position and size hints set, otherwise invisible at (0,0).

.. code-block:: c

   xysizer_attach(sizer, filler_create(), 10, 10, 50, 50);
