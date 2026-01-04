Widget Base
===========

The :c:struct:`widget` structure wraps a :c:struct:`component` and provides the foundation for
all leaf-node GUI elements. It adds an ID system for widget lookup and manages
callback routing to specialized widget implementations.

Source: ``src/game/gui/widget.c``

.. seealso:: :doc:`/api/game/gui/widget` for the API reference.


Overview
--------

The widget layer provides:

- **Callback indirection** - Routes component callbacks to widget-specific implementations
- **ID system** - Assigns numeric IDs for later lookup via :c:func:`gui_frame_find`
- **Specialization pointer** - Points to widget-specific data (button, label, etc.)
- **Default state support** - Enables disable/select/focus by default

.. note::

   The :c:struct:`widget` is an **opaque type** - its structure is defined in
   ``widget.c``, not the header. You interact with it through the public API.


Architecture
------------

The widget layer sits between the component and specialized widget types:

- **Component** (header = ``WIDGET_MAGIC`` / ``0x8BADF00D``)

  - ``obj`` → points to widget struct
  - Callbacks: render, event, action, focus, layout, tick, free, init, find

- **Widget** (allocated in :c:func:`widget_create`)

  - ``obj`` → points to specialized widget data
  - ``id`` → numeric ID for lookup (-1 by default)
  - Callbacks: render, event, action, focus, layout, tick, init, free

- **Specialized Widget** (button, label, gauge, etc.)

  - Widget-specific data (text, callbacks, state, etc.)


Creation Flow
-------------

:c:func:`widget_create` performs:

1. Create component with widget magic header
2. Enable all state support flags (disable, select, focus)
3. Allocate widget struct
4. Set default ID to -1
5. Store widget pointer in component's ``obj``
6. Install routing callbacks (widget_render, widget_action, etc.)
7. Return the component


Callback Routing
----------------

Widget callbacks route through two layers. The component-level callbacks
(installed by :c:func:`widget_create`) delegate to widget-level callbacks
(set by specific widget constructors):

.. graphviz::
   :caption: Two-Layer Callback Routing

   digraph routing {
      rankdir=TB
      node [shape=box, style=filled]

      subgraph cluster_public {
         label="Public API"
         style=filled
         fillcolor=lightyellow
         api [label="component_render(c)", fillcolor=lightyellow]
      }

      subgraph cluster_component {
         label="Component Layer"
         style=filled
         fillcolor=lightblue
         check1 [label="c->render != NULL?", shape=diamond, fillcolor=white]
         widget_render [label="widget_render(c)", fillcolor=lightblue]
      }

      subgraph cluster_widget {
         label="Widget Layer"
         style=filled
         fillcolor=lightgreen
         get_obj [label="widget_get_obj(c)", fillcolor=lightgreen]
         check2 [label="has render cb?", shape=diamond, fillcolor=white]
         specific [label="button_render(c)", fillcolor=palegreen]
      }

      api -> check1
      check1 -> widget_render [label="yes"]
      widget_render -> get_obj
      get_obj -> check2
      check2 -> specific [label="yes"]
   }

**Routing Flow:**

1. Public API call (e.g., :c:func:`component_render`)
2. Component layer checks if ``c->render != NULL``
3. If set, calls ``widget_render`` (installed by :c:func:`widget_create`)
4. Widget layer retrieves widget data via :c:func:`widget_get_obj`
5. Widget layer checks if widget has render callback set
6. If set, calls widget-specific render (e.g., ``button_render``)


ID Lookup Flow
--------------

The widget's internal :c:func:`widget_find` callback enables lookup by ID:

1. :c:func:`gui_frame_find` called with frame and target ID
2. Frame searches component tree via :c:func:`component_find`
3. For sizers: iterates children recursively
4. For widgets: calls :c:func:`widget_find` which checks :c:func:`widget_get_id`
5. If ``widget_get_id(c) == id``, returns the component
6. Otherwise returns NULL

**Usage example:**

.. code-block:: c

   // During tree construction - assign an ID
   component *selector = textselector_create("RESOLUTION", NULL,
                                             on_resolution_change, NULL);
   widget_set_id(selector, WIDGET_ID_RESOLUTION);
   menu_attach(options_menu, selector);

   // Later - find by ID
   component *res = gui_frame_find(frame, WIDGET_ID_RESOLUTION);
   if(res != NULL) {
       textselector_set_pos(res, new_resolution);
   }


Default State Support
---------------------

:c:func:`widget_create` enables all state flags by default:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Flag
     - Default
     - Purpose
   * - :c:member:`component.supports_disable`
     - ``true``
     - Widget can be grayed out
   * - :c:member:`component.supports_select`
     - ``true``
     - Widget can be highlighted in menu
   * - :c:member:`component.supports_focus`
     - ``true``
     - Widget can receive keyboard focus

Individual widget types override these as needed:

.. code-block:: c

   // Example: Gauge is display-only, supports no states
   component *gauge_create(...) {
       component *c = widget_create();
       component_set_supports(c, false, false, false);
       // ...
   }

   // Example: Label supports disable but not select/focus
   component *label_create(...) {
       component *c = widget_create();
       component_set_supports(c, true, false, false);
       // ...
   }


Creating a Custom Widget
------------------------

To create a new widget type:

.. code-block:: c

   typedef struct my_widget {
       // Widget-specific data
       char *text;
       my_widget_cb callback;
       void *userdata;
   } my_widget;

   static void my_widget_render(component *c) {
       my_widget *local = widget_get_obj(c);
       const gui_theme *theme = component_get_theme(c);

       vga_index color = component_is_selected(c)
           ? theme->text.active_color
           : theme->text.primary_color;

       // Draw widget at c->x, c->y with size c->w, c->h
   }

   static int my_widget_action(component *c, int action) {
       my_widget *local = widget_get_obj(c);

       if(action == ACT_KICK || action == ACT_PUNCH) {
           if(local->callback) {
               local->callback(c, local->userdata);
           }
           return 0;  // Handled
       }
       return 1;  // Not handled
   }

   static void my_widget_free(component *c) {
       my_widget *local = widget_get_obj(c);
       omf_free(local->text);
       omf_free(local);
   }

   component *my_widget_create(const char *text, my_widget_cb cb, void *ud) {
       component *c = widget_create();

       // Configure state support if different from defaults
       // component_set_supports(c, true, true, false);

       // Allocate and initialize widget data
       my_widget *local = omf_calloc(1, sizeof(my_widget));
       local->text = strdup(text);
       local->callback = cb;
       local->userdata = ud;
       widget_set_obj(c, local);

       // Set callbacks
       widget_set_render_cb(c, my_widget_render);
       widget_set_action_cb(c, my_widget_action);
       widget_set_free_cb(c, my_widget_free);

       return c;
   }
