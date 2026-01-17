GUI System Architecture
=======================

OpenOMF uses a custom callback-based GUI system built on a tree structure of components. The system
is designed around the concept of widgets (leaf nodes) and sizers (container nodes), with a unified
component interface providing rendering, event handling, and layout functionality.

Component Hierarchy
-------------------

The GUI system is built on a two-level type system:

.. graphviz::

   digraph component_hierarchy {
       rankdir=TB
       node [shape=box, fontname="monospace"]

       component [label="component\n(base struct)"]

       widget [label="widget\n(leaf node)"]
       sizer [label="sizer\n(container node)"]

       component -> widget [label="WIDGET_MAGIC\n0x8BADF00D"]
       component -> sizer [label="SIZER_MAGIC\n0xDEADBEEF"]

       button [label="button"]
       label [label="label"]
       textinput [label="textinput"]
       gauge [label="gauge"]

       xysizer [label="xysizer"]
       menu [label="menu"]

       widget -> button
       widget -> label
       widget -> textinput
       widget -> gauge

       sizer -> xysizer
       sizer -> menu
   }

**Widgets** are leaf nodes representing interactive elements like buttons, labels, and text inputs.
They cannot have children and are created via ``widget_create()``.

**Sizers** are container nodes that manage layout and hold a vector of child components. They handle
event propagation to children and manage the layout algorithm. Created via ``sizer_create()``.

Each component uses a magic header value (``0x8BADF00D`` for widgets, ``0xDEADBEEF`` for sizers) for
runtime type safety checks.

Callback Architecture
---------------------

Components use function pointers for lifecycle operations, allowing specialization without inheritance:

============  ==================================================================
Callback      Purpose
============  ==================================================================
``render``    Draw the component to screen
``event``     Handle raw SDL2 events (keyboard, mouse)
``action``    Handle abstract game actions (ACT_UP, ACT_KICK, etc.)
``focus``     Called when focus state changes
``layout``    Calculate and apply position/size
``tick``      Periodic updates (animations, state changes)
``free``      Cleanup and resource deallocation
``init``      Initialize with theme, compute size hints
``find``      Search for child by ID
============  ==================================================================

Component Lifecycle
-------------------

.. graphviz::

   digraph lifecycle {
       rankdir=LR
       node [shape=box, fontname="monospace"]

       create [label="Create"]
       attach [label="Attach"]
       init [label="Init"]
       layout [label="Layout"]
       runtime [label="Runtime\n(tick/render/event)"]
       destroy [label="Destroy"]

       create -> attach -> init -> layout -> runtime -> destroy
   }

1. **Create**: Base component allocated, specialization struct created
2. **Attach**: Child added to parent's vector via ``sizer_attach()``
3. **Init**: Theme propagated, size hints computed
4. **Layout**: Final position and size calculated recursively
5. **Runtime**: Main loop calls ``tick()``, ``render()``, and event handlers
6. **Destroy**: ``component_free()`` recursively frees all children

Components have no position or size until layout is called. Size hints set during init are
suggestions that the layout algorithm uses to compute final dimensions.

Event Handling
--------------

The system uses two levels of input handling:

**SDL Events** are raw hardware events (keyboard, mouse) passed via ``component_event()``. These
allow direct access to input state.

**Actions** are abstract game inputs (ACT_UP, ACT_DOWN, ACT_PUNCH, ACT_KICK, ACT_ESC) passed via
``component_action()``. This provides controller/keyboard-agnostic input handling.

Events propagate from parent to children. A return value of 0 indicates the event was consumed,
stopping further propagation.

Rendering
---------

Rendering follows a recursive depth-first traversal of the component tree:

1. Sizers iterate through children and render each
2. Widgets call their specialized render callback
3. Sizers support opacity (0.0-1.0) for fade effects

The render callback draws the component at its computed position (``c->x``, ``c->y``) with
computed dimensions (``c->w``, ``c->h``).

Layout System
-------------

Layout is a two-phase process:

1. **Init phase**: Components set size hints (``w_hint``, ``h_hint``) based on content
2. **Layout phase**: Parent sizers compute final positions for children

Different sizer types implement different layout algorithms:

- **xysizer**: Absolute positioning using hints directly as final values
- **menu**: Vertical list with selection tracking and navigation

Size hints are set during ``init``. For example, a button measures its text width and sets
``w_hint`` accordingly.

GUI Frame
---------

A ``gui_frame`` is the top-level container that holds:

- The root component (typically a sizer)
- Theme configuration
- Frame dimensions

The frame coordinates initialization and layout:

.. code-block:: c

   gui_frame_layout(frame);  // Triggers init + layout on entire tree

Theming
-------

The GUI uses a single theme per frame containing:

- **Dialog theme**: Border colors
- **Text theme**: Font selection, colors (primary, secondary, active, inactive, disabled, shadow)

Theme is propagated during ``init`` and accessed via ``component_get_theme()`` during rendering.
