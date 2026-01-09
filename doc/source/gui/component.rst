Component
=========

The :c:struct:`component` structure is the base class for all GUI elements. Every widget
and sizer is built on top of this foundation.

Source: ``src/game/gui/component.c``

.. seealso:: :doc:`/api/game/gui/component` for the API reference.


Overview
--------

The component abstraction provides:

- Position and size management
- Layout hints for sizers
- State flags (selected, disabled, focused)
- Callback-based polymorphism
- Help text support
- Theme integration

.. note::

   Components don't have position or size until :c:func:`component_layout` has been called.
   The layout call for a sizer will cascade to all its children.


Memory Layout
-------------

The GUI uses a layered composition pattern where each layer points to the next:

.. graphviz::
   :caption: Memory Layout - Pointer Chain

   digraph memory {
      rankdir=LR
      node [shape=record, style=filled, fontname="monospace", fontsize=10]

      component [label="{component|header\lx, y, w, h\lhints\lstate flags\ltheme\lparent\lcallbacks\l|obj}", fillcolor=lightyellow]

      widget [label="{widget|id\lcallbacks\l|obj}", fillcolor=lightgreen]
      button [label="{button|text\lclick_cb\luserdata\l}", fillcolor=palegreen]

      sizer [label="{sizer|objs[ ]\lopacity\lcallbacks\l|obj}", fillcolor=lightpink]
      menu [label="{menu|selected\lsubmenu\lbg_surfaces\l}", fillcolor=mistyrose]

      component:obj -> widget [label="widgets"]
      widget:obj -> button

      component:obj -> sizer [label="sizers", style=dashed]
      sizer:obj -> menu [style=dashed]
   }

**Base Component** (:c:struct:`component`):

- ``header`` - Magic number for type checking (``COMPONENT_MAGIC``)
- ``x``, ``y``, ``w``, ``h`` - Position and size (set by layout)
- ``x_hint``, ``y_hint``, ``w_hint``, ``h_hint`` - Layout hints
- ``obj`` - Pointer to widget or sizer data
- State flags and support flags
- ``help`` - Help text object
- ``theme`` - Pointer to GUI theme
- ``parent`` - Parent component pointer
- Callback function pointers

**Widget Layer** (:c:struct:`widget`, accessed via ``component->obj``):

- ``obj`` - Pointer to concrete widget data (button, label, etc.)
- ``id`` - Widget ID for lookup via :c:func:`gui_frame_find`
- Widget-specific callbacks

**Sizer Layer** (:c:struct:`sizer`, accessed via ``component->obj``):

- ``obj`` - Pointer to concrete sizer data (menu, xysizer, etc.)
- ``objs`` - Vector of child components
- ``opacity`` - For fade effects
- Sizer-specific callbacks

**Concrete Data** (e.g., :c:struct:`button`, :c:struct:`menu`):

- Widget/sizer-specific fields (text, callbacks, state, etc.)


Lifecycle
---------

**Creation:**

Components are created via :c:func:`widget_create` or :c:func:`sizer_create`, which
allocate the component and set up the magic header for type checking.

**Initialization** (:c:func:`component_init`):

1. Store the theme pointer via :c:func:`component_set_theme`
2. Call the ``init`` callback if set

**Layout** (:c:func:`component_layout`):

1. Set position and size from parameters
2. Call the ``layout`` callback if set

**Cleanup** (:c:func:`component_free`):

1. Return early if component is NULL
2. Unfocus the component if it was focused
3. Call the ``free`` callback if set
4. Free help text if present
5. Free the component structure


State Management
----------------

Components have three independent state flags, each gated by a support flag:

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - State
     - Functions
     - Description
   * - Focused
     - :c:func:`component_focus`, :c:func:`component_is_focused`
     - Has keyboard/input focus. Focus changes invoke the focus callback.
   * - Selected
     - :c:func:`component_select`, :c:func:`component_is_selected`
     - Highlighted in a menu. Affects visual appearance.
   * - Disabled
     - :c:func:`component_disable`, :c:func:`component_is_disabled`
     - Grayed out and non-interactive. Skipped during menu navigation.

Use :c:func:`component_set_supports` to configure which states a component supports.
State setters check the corresponding support flag before modifying state.


Usage Example
-------------

Typically you don't use :c:func:`component_create` directly. Instead, use
:c:func:`widget_create` or :c:func:`sizer_create` which set up the component correctly:

.. code-block:: c

   // Creating a custom widget
   component *my_widget_create(void) {
       component *c = widget_create();  // Creates component with WIDGET_MAGIC

       // Configure state support
       component_set_supports(c, true, true, true);  // disable, select, focus

       // Allocate and attach widget-specific data
       my_widget *local = omf_calloc(1, sizeof(my_widget));
       // ... initialize local data ...
       widget_set_obj(c, local);

       // Set callbacks
       widget_set_render_cb(c, my_widget_render);
       widget_set_action_cb(c, my_widget_action);
       widget_set_free_cb(c, my_widget_free);
       widget_set_init_cb(c, my_widget_init);
       widget_set_layout_cb(c, my_widget_layout);

       return c;
   }


The component is then used within a GUI tree:

.. code-block:: c

   // Build tree
   component *menu = menu_create();
   menu_attach(menu, my_widget_create());
   menu_attach(menu, button_create("OK", NULL, false, false, cb, NULL));

   // Initialize and layout via frame
   gui_frame *frame = gui_frame_create(&theme, 0, 0, 320, 200);
   gui_frame_set_root(frame, menu);
   gui_frame_layout(frame);

   // Game loop
   while(running) {
       gui_frame_tick(frame);
       gui_frame_render(frame);
       // ... handle events via gui_frame_event/action ...
   }

   // Cleanup
   gui_frame_free(frame);  // Frees entire tree
