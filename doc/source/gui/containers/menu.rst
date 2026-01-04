Menu
====

A sizer that provides keyboard navigation, selection management, help text display,
and optional submenu support. This is the primary container for game menus.

Source: ``src/game/gui/menu.c``

.. seealso:: :doc:`/api/game/gui/menu` for the API reference.


Features
--------

- Vertical or horizontal item layout
- Keyboard navigation (up/down or left/right)
- Selection tracking with visual feedback
- Help text display for selected items
- Submenu support with automatic delegation
- Optional background rendering
- Centering and padding options


Navigation Actions
------------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Action
     - Behavior
   * - ``ACT_UP``
     - Previous item (vertical menus)
   * - ``ACT_DOWN``
     - Next item (vertical menus)
   * - ``ACT_LEFT``
     - Previous item (horizontal menus)
   * - ``ACT_RIGHT``
     - Next item (horizontal menus)
   * - ``ACT_KICK``
     - Activate selected item
   * - ``ACT_PUNCH``
     - Activate selected item
   * - ``ACT_ESC``
     - Select last item; if already selected, set finished


Call Chains
-----------

**Layout** (``menu_layout``):

1. Calculate available space (width for horizontal, height for vertical)
2. Calculate reserved space by summing children with size hints
3. Calculate space per unhinted item from remaining space
4. Select first non-disabled item via :c:func:`menu_select`
5. Calculate centering offset if all items have hints
6. For each child: determine size from hint or calculated, call :c:func:`component_layout`, advance offset
7. Create background surfaces if background is enabled

**Action** (``menu_action``):

.. list-table::
   :widths: 25 75

   * - Check
     - Action
   * - Submenu active?
     - Delegate action to submenu via :c:func:`component_action`, return
   * - ESC action?
     - Select last item; if was already selected or is submenu, mark finished and convert to PUNCH action
   * - Navigation action?
     - Deselect old item via :c:func:`component_select`, cycle to next non-disabled item, play navigation sound, select new item, return 0
   * - Otherwise
     - Delegate to selected child via :c:func:`component_action`

**Render** (``menu_render``):

1. If submenu is active and not finished, render only the submenu and return
2. Draw transparent background (bg1) if set
3. Draw solid background (bg2) if set
4. For each child (with index i):

   - Call :c:func:`component_render` on child
   - If i equals selected index and child has help text, draw help background and help text


Usage Examples
--------------

**Basic Vertical Menu:**

.. code-block:: c

   component *menu = menu_create();
   menu_attach(menu, button_create("Start Game", "Begin adventure", false, false, start_cb, NULL));
   menu_attach(menu, button_create("Options", "Configure settings", false, false, options_cb, NULL));
   menu_attach(menu, button_create("Quit", "Exit game", false, false, quit_cb, NULL));


**Horizontal Menu:**

.. code-block:: c

   component *menu = menu_create();
   menu_set_horizontal(menu, true);
   menu_set_padding(menu, 20);
   menu_attach(menu, button_create("P1", NULL, false, false, cb, NULL));
   menu_attach(menu, button_create("P2", NULL, false, false, cb, NULL));


**Menu without Background:**

.. code-block:: c

   component *menu = menu_create();
   menu_set_background(menu, false);


**Centered Menu:**

.. code-block:: c

   component *menu = menu_create();
   menu_set_centered(menu, true);
   // All items must have size hints for centering to work


**Custom Help Position:**

.. code-block:: c

   component *menu = menu_create();
   menu_set_help_pos(menu, 10, 180, 300, 20);
   menu_set_help_text_settings(menu, FONT_SMALL, TEXT_ALIGN_CENTER, COLOR_LIGHT_BLUE);


**Submenu:**

.. code-block:: c

   void on_options(component *c, void *userdata) {
       component *parent = c->parent;
       component *submenu = create_options_menu();
       menu_set_submenu(parent, submenu);
   }

   void on_submenu_done(component *menu, component *submenu) {
       // Submenu finished, process results
   }

   component *main = menu_create();
   menu_set_submenu_done_cb(main, on_submenu_done);
   menu_attach(main, button_create("Options", NULL, false, false, on_options, NULL));
