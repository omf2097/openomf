Widget Overview
===============

Widgets are leaf nodes in the GUI component tree. They display content, accept
user input, and implement specific UI behaviors. All widgets are built on top
of the :c:struct:`widget` wrapper structure.


Widget Hierarchy
----------------

All widgets inherit from the widget base (magic header ``0x8BADF00D``):

.. graphviz::
   :caption: Widget Type Hierarchy

   digraph widgets {
      rankdir=TB
      node [shape=box, style="rounded,filled"]

      widget [label="widget base\n(0x8BADF00D)", fillcolor=lightyellow]

      subgraph cluster_text {
         label="Text-Based"
         style=filled
         fillcolor=lightgreen
         button [label="button", fillcolor=palegreen]
         label_w [label="label", fillcolor=palegreen]
         textinput [label="textinput", fillcolor=palegreen]
         textselector [label="textselector", fillcolor=palegreen]
         textslider [label="textslider", fillcolor=palegreen]
      }

      subgraph cluster_visual {
         label="Visual"
         style=filled
         fillcolor=lightpink
         gauge [label="gauge", fillcolor=mistyrose]
         progressbar [label="progressbar", fillcolor=mistyrose]
         spritebutton [label="spritebutton", fillcolor=mistyrose]
         spriteimage [label="spriteimage", fillcolor=mistyrose]
         portrait [label="portrait", fillcolor=mistyrose]
      }

      subgraph cluster_special {
         label="Special"
         style=filled
         fillcolor=lightgray
         filler [label="filler", fillcolor=gainsboro]
         trnselect [label="trnselect", fillcolor=gainsboro]
      }

      widget -> button
      widget -> label_w
      widget -> textinput
      widget -> textselector
      widget -> textslider
      widget -> gauge
      widget -> progressbar
      widget -> spritebutton
      widget -> spriteimage
      widget -> portrait
      widget -> filler
      widget -> trnselect
   }

**Text-Based Widgets:**

- :doc:`button` - Clickable text button
- :doc:`label` - Static text display
- :doc:`textinput` - Text entry field with cursor
- :doc:`textselector` - Dropdown-style option selector
- :doc:`textslider` - Visual slider with discrete positions

**Visual Widgets:**

- :doc:`gauge` - Lit/unlit indicator bar
- :doc:`progressbar` - Animated progress indicator
- :doc:`spritebutton` - Clickable image button
- :doc:`spriteimage` - Static image display
- :doc:`portrait` - Fighter portrait display

**Special Widgets:**

- :doc:`filler` - Empty space placeholder
- :doc:`trnselect` - Tournament selector


Widget Categories
-----------------

**Interactive Widgets**
   These respond to user input and typically have action callbacks:

   - :doc:`button` - Clickable text button
   - :doc:`textinput` - Text entry field with cursor
   - :doc:`textselector` - Dropdown-style option selector
   - :doc:`textslider` - Visual slider with discrete positions
   - :doc:`spritebutton` - Clickable image button

**Display Widgets**
   These show information but don't accept input:

   - :doc:`label` - Static text display
   - :doc:`gauge` - Lit/unlit indicator bar
   - :doc:`progressbar` - Animated progress indicator
   - :doc:`spriteimage` - Static image display
   - :doc:`portrait` - Fighter portrait display

**Layout Widgets**
   These assist with layout but have no visual representation:

   - :doc:`filler` - Empty space placeholder


Widget State Support
--------------------

Each widget type declares which states it supports:

.. list-table::
   :header-rows: 1
   :widths: 25 15 15 15 30

   * - Widget
     - Disable
     - Select
     - Focus
     - Notes
   * - Button
     - Yes
     - Yes
     - Yes
     - Full interaction
   * - Label
     - Yes
     - No
     - No
     - Display only
   * - TextInput
     - Yes
     - Yes
     - Yes
     - Needs focus for input
   * - TextSelector
     - Yes
     - Yes
     - Yes
     - Full interaction
   * - TextSlider
     - Yes
     - Yes
     - Yes
     - Full interaction
   * - Gauge
     - No
     - No
     - No
     - Display only
   * - ProgressBar
     - No
     - No
     - No
     - Display only
   * - SpriteButton
     - Yes
     - Yes
     - Yes
     - Has focus callback
   * - SpriteImage
     - Yes
     - No
     - No
     - Display only
   * - Portrait
     - No
     - No
     - No
     - Display only
   * - Filler
     - Yes
     - No
     - No
     - Invisible


Common Widget Patterns
----------------------

**Creation Pattern:**

.. code-block:: c

   component *my_widget_create(...) {
       component *c = widget_create();

       // Set state support
       component_set_supports(c, true, true, true);

       // Allocate local data
       my_widget *local = omf_calloc(1, sizeof(my_widget));
       // ... initialize local ...
       widget_set_obj(c, local);

       // Set callbacks
       widget_set_render_cb(c, my_widget_render);
       widget_set_action_cb(c, my_widget_action);
       widget_set_free_cb(c, my_widget_free);
       widget_set_init_cb(c, my_widget_init);
       widget_set_layout_cb(c, my_widget_layout);

       return c;
   }


**Render Pattern:**

.. code-block:: c

   static void my_widget_render(component *c) {
       my_widget *local = widget_get_obj(c);
       const gui_theme *theme = component_get_theme(c);

       // Select color based on state
       vga_index color;
       if(component_is_selected(c)) {
           color = theme->text.active_color;
       } else if(component_is_disabled(c)) {
           color = theme->text.disabled_color;
       } else {
           color = theme->text.inactive_color;
       }

       // Draw at component position
       text_set_color(local->text, color);
       text_draw(local->text, c->x, c->y);
   }


**Action Pattern:**

.. code-block:: c

   static int my_widget_action(component *c, int action) {
       my_widget *local = widget_get_obj(c);

       if(action == ACT_KICK || action == ACT_PUNCH) {
           if(local->callback) {
               local->callback(c, local->userdata);
           }
           audio_play_sound(20, 0.5f, 0.0f, 0);
           return 0;  // Handled
       }

       return 1;  // Not handled
   }
