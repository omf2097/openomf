GUI Overview
============

The GUI system is built on three core abstractions:

- **Component** - Base class for all GUI elements with callbacks for rendering, events, and layout
- **Widget** - Leaf nodes (buttons, labels, inputs) that wrap specialized objects
- **Sizer** - Container nodes that hold and layout child components

.. graphviz::
   :caption: Core Architecture

   digraph architecture {
      rankdir=TB
      node [shape=box, style=filled, fillcolor=lightblue]
      newrank=true

      Component [label="Component\n(base class)", fillcolor=lightyellow]

      subgraph cluster_containers {
         label="Containers (0xDEADBEEF)"
         style=filled
         fillcolor=lightgreen
         Sizer
         Menu
         TrnMenu [label="TrnMenu"]
         XYSizer
      }

      subgraph cluster_widgets {
         label="Widgets (0x8BADF00D)"
         style=filled
         fillcolor=lightpink
         node [fillcolor=lightpink]
         Widget
         w1 [label="Button\nLabel\nTextInput"]
         w2 [label="Gauge\nProgressBar"]
         w3 [label="TextSelector\nTextSlider"]
         w4 [label="SpriteButton\nSpriteImage\nPortrait"]
      }

      Component -> Sizer
      Component -> Widget
      Sizer -> Menu
      Sizer -> TrnMenu
      Sizer -> XYSizer
      Widget -> w1
      Widget -> w2
      Widget -> w3
      Widget -> w4
      {rank=same; w1; w2; w3; w4}
   }


Component Tree Example
----------------------

A typical menu structure forms a tree:

.. graphviz::
   :caption: Example Menu Tree

   digraph tree {
      rankdir=TB
      node [shape=box, style=filled]

      frame [label="gui_frame", fillcolor=lightyellow]
      menu [label="Menu (sizer)", fillcolor=lightgreen]
      btn1 [label="Button: Start", fillcolor=lightpink]
      btn2 [label="Button: Options", fillcolor=lightpink]
      btn3 [label="Button: Quit", fillcolor=lightpink]

      frame -> menu
      menu -> btn1
      menu -> btn2
      menu -> btn3
   }


Key Concepts
------------

**Callbacks**
   Every component can have up to 9 callback functions: render, event, action, focus,
   layout, tick, free, init, and find. These enable polymorphic behavior.

**Magic Headers**
   Runtime type checking via magic numbers in the component header field:

   - ``0xDEADBEEF`` - Identifies sizer components
   - ``0x8BADF00D`` - Identifies widget components

**Two-Phase Initialization**
   Components are first created, then initialized and laid out via :c:func:`gui_frame_layout`.
   This separates construction from sizing/positioning.

**Event Pathways**
   Two parallel systems: raw SDL events and abstract action codes from the controller.
