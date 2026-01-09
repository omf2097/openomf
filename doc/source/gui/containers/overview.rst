Container Overview
==================

Containers (sizers) are non-leaf nodes that hold and layout child components.
They manage component trees, handle event routing, and implement layout algorithms.


Container Hierarchy
-------------------

All containers inherit from the :c:struct:`sizer` base (magic header ``0xDEADBEEF``):

.. graphviz::
   :caption: Container Inheritance Hierarchy

   digraph containers {
      rankdir=TB
      node [shape=box, style="rounded,filled", fillcolor=lightblue]

      sizer [label="sizer\n(base)", fillcolor=lightyellow]
      menu [label="menu\nNavigation + help"]
      trnmenu [label="trnmenu\nAnimations + hand"]
      xysizer [label="xysizer\nAbsolute positioning"]
      dialog [label="dialog\n(wraps gui_frame)", style="rounded,filled,dashed", fillcolor=lightgray]

      sizer -> menu
      sizer -> trnmenu
      sizer -> xysizer

      dialog -> sizer [style=dashed, label="uses internally"]
   }

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Container
     - Description
   * - :doc:`sizer`
     - Base container with child vector
   * - :doc:`menu`
     - Navigation + help text support
   * - :doc:`trnmenu`
     - Animations + hand cursor
   * - :doc:`xysizer`
     - Absolute positioning

Note: :doc:`dialog` is a higher-level construct that wraps a :c:struct:`gui_frame`
rather than being a direct sizer subtype.


Container Categories
--------------------

**Navigation Containers**
   Handle selection, focus, and input routing:

   - :doc:`menu` - Vertical/horizontal menu with keyboard navigation
   - :doc:`trnmenu` - Tournament menu with animation and hand cursor

**Layout Containers**
   Position child components:

   - :doc:`sizer` - Base container with child vector
   - :doc:`xysizer` - Absolute positioning based on hints

**High-Level Containers**
   Pre-built UI patterns:

   - :doc:`dialog` - Modal dialog with buttons


Container Responsibilities
--------------------------

All containers handle:

1. **Child Management**: Adding, iterating, freeing children
2. **Layout**: Positioning children based on hints and available space
3. **Event Routing**: Passing events to appropriate children
4. **Rendering**: Drawing children (and backgrounds)
5. **Lifecycle**: Initializing and freeing children


Common Operations
-----------------

**Adding Children:**

.. code-block:: c

   // Menu
   menu_attach(menu, button_create("Item", NULL, false, false, cb, NULL));

   // XYSizer (with position)
   xysizer_attach(sizer, widget, 10, 20, 50, 30);

   // Generic sizer
   sizer_attach(sizer, child);


**Querying Children:**

.. code-block:: c

   // Get child count
   int count = sizer_size(container);

   // Get specific child
   component *child = sizer_get(container, index);

   // Iterate children
   iterator it;
   component **tmp;
   sizer_begin_iterator(container, &it);
   foreach(it, tmp) {
       // Use *tmp
   }


Event Flow
----------

When an action is passed to a container via :c:func:`component_action`:

1. Sizer routes to specialization's action callback
2. If container has an active submenu, delegate to submenu
3. Otherwise, handle navigation actions (if applicable)
4. For non-navigation actions, delegate to selected child


Layout Algorithms
-----------------

**Menu (Linear):**
   Distributes children along one axis (vertical or horizontal) with
   optional centering and padding.

**XYSizer (Absolute):**
   Positions each child at its x_hint/y_hint coordinates with
   w_hint/h_hint dimensions.

**TrnMenu (Absolute with Animation):**
   Like XYSizer but with selection navigation, hand cursor, and
   fading transitions.
