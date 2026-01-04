Architecture
============

This page documents the internal architecture, call chains, and data flow
within the GUI system.


Callback System
---------------

The GUI system uses a callback-based polymorphism pattern. Each component stores
function pointers that are invoked by the base component functions.

.. list-table:: Public API to Callback Mapping
   :header-rows: 1
   :widths: 40 60

   * - Public Function
     - Invokes Callback
   * - :c:func:`component_render`
     - ``render`` callback
   * - :c:func:`component_tick`
     - ``tick`` callback
   * - :c:func:`component_event`
     - ``event`` callback
   * - :c:func:`component_action`
     - ``action`` callback
   * - :c:func:`component_layout`
     - ``layout`` callback
   * - :c:func:`component_init`
     - ``init`` callback (after setting theme)
   * - :c:func:`component_free`
     - ``free`` callback (after unfocusing)
   * - :c:func:`component_focus`
     - ``focus`` callback
   * - :c:func:`component_find`
     - ``find`` callback


Inheritance Hierarchy
---------------------

The inheritance is implemented through composition. The base :c:struct:`component`
structure contains an ``obj`` pointer that points to type-specific data.

.. graphviz::
   :caption: Component Composition Pattern

   digraph inheritance {
      rankdir=TB
      node [shape=box, style=filled, fontname="monospace"]

      subgraph cluster_component {
         label="component"
         style=filled
         fillcolor=lightyellow

         comp [label="header\nposition (x,y,w,h)\nhints\nstate flags\ncallbacks\nobj →", fillcolor=lightyellow]
      }

      subgraph cluster_widget {
         label="Widget Path"
         style=filled
         fillcolor=lightgreen

         widget [label="widget\n─────────\nid\ncallbacks\nobj →", fillcolor=lightgreen]
         button [label="button\n─────────\ntext\nclick_cb\nuserdata", fillcolor=palegreen]
         label_w [label="label\n─────────\ntext\ncolor", fillcolor=palegreen]
      }

      subgraph cluster_sizer {
         label="Sizer Path"
         style=filled
         fillcolor=lightpink

         sizer [label="sizer\n─────────\nobjs[ ]\nopacity\ncallbacks\nobj →", fillcolor=lightpink]
         menu [label="menu\n─────────\nselected\nsubmenu\nbackground", fillcolor=mistyrose]
         xysizer [label="xysizer\n─────────\n(no extra data)", fillcolor=mistyrose]
      }

      comp -> widget [label="WIDGET_MAGIC\n0x8BADF00D"]
      comp -> sizer [label="SIZER_MAGIC\n0xDEADBEEF"]

      widget -> button
      widget -> label_w

      sizer -> menu
      sizer -> xysizer
   }

**Component Structure:**

The base :c:struct:`component` contains:

- Position and size (``x``, ``y``, ``w``, ``h``)
- State flags (``is_focused``, ``is_selected``, ``is_disabled``)
- Capability flags (``supports_focus``, ``supports_select``, ``supports_disable``)
- Hint values (``x_hint``, ``y_hint``, ``w_hint``, ``h_hint``)
- Callback function pointers
- ``obj`` pointer to type-specific data

**Widget Layer:**

Created by :c:func:`widget_create`, the widget layer wraps a component and adds:

- Widget-specific ``obj`` pointer for concrete widget data (button, label, etc.)
- Widget ID for lookup via :c:func:`gui_frame_find`
- Widget-specific callbacks that delegate to the concrete implementation

**Sizer Layer:**

Created by :c:func:`sizer_create`, the sizer layer wraps a component and adds:

- Vector of child components (``objs``)
- Sizer-specific ``obj`` pointer for concrete sizer data (menu, xysizer, etc.)
- Opacity value for fade effects
- Sizer-specific callbacks that iterate over children


Operation Call Chains
---------------------

Initialization
^^^^^^^^^^^^^^

When :c:func:`gui_frame_layout` is called, it triggers a two-phase initialization:

1. **Init Phase**: :c:func:`component_init` is called on the root, which:

   - Stores the theme pointer via :c:func:`component_set_theme`
   - Invokes the ``init`` callback

2. **Layout Phase**: :c:func:`component_layout` is called on the root, which:

   - Invokes the ``layout`` callback with position and size

**Sizer Init Behavior:**

When a sizer's init callback runs (e.g., ``sizer_init``):

1. Calls the specialization's init callback if set (e.g., menu-specific init)
2. Iterates over all children calling :c:func:`component_init` on each

**Widget Init Behavior:**

When a widget's init callback runs (e.g., ``widget_init``):

1. Calls the widget-specific init callback (e.g., ``button_init``)
2. The specific init typically sets font from theme and calculates size hints


Rendering
^^^^^^^^^

Rendering propagates through the component tree via :c:func:`gui_frame_render`:

1. Frame calls :c:func:`component_render` on the root
2. Root's render callback is invoked
3. For sizers: the specialization renders background, then iterates children
4. For widgets: the specific render callback draws the widget

**Menu Render Sequence** (``menu_render``):

.. list-table::
   :widths: 10 90

   * - 1
     - Check if submenu is active and not finished; if so, render only the submenu and return
   * - 2
     - Draw background surfaces (transparent layer, then solid layer)
   * - 3
     - Iterate children, calling :c:func:`component_render` on each
   * - 4
     - For the selected child with help text, draw help background and text


Tick (Animation)
^^^^^^^^^^^^^^^^

The tick function updates animation state each frame via :c:func:`gui_frame_tick`:

1. Frame calls :c:func:`component_tick` on the root
2. Sizers call their specialization tick, then iterate children
3. Widgets call their specific tick callback

**Animation Examples:**

- **ProgressBar**: Animates from current to target value (1% per tick)
- **SpriteButton**: Decrements ``active_ticks`` counter for visual feedback
- **TrnMenu**: Handles opacity fading and hand cursor movement


Event Handling
^^^^^^^^^^^^^^

Two parallel event pathways exist:

**SDL Event Path** (for keyboard/mouse input):

:c:func:`gui_frame_event` → :c:func:`component_event` → sizer/widget event callback

- Sizers delegate to submenu if active, otherwise to selected child
- Widgets handle SDL events (e.g., TextInput handles text input and clipboard)

**Action Path** (for game actions like ACT_UP, ACT_KICK):

:c:func:`gui_frame_action` → :c:func:`component_action` → sizer/widget action callback

- Sizers delegate to submenu if active, otherwise handle navigation or delegate to selected child
- Widgets handle activation (e.g., Button triggers click callback on ACT_KICK/ACT_PUNCH)

**Menu Action Processing** (in ``menu_action``):

.. list-table::
   :widths: 25 75

   * - Submenu check
     - If submenu is active and not finished, delegate action to submenu
   * - ESC handling
     - Select last item; if already selected or is submenu, mark finished
   * - Navigation
     - For UP/DOWN (vertical) or LEFT/RIGHT (horizontal): deselect old, cycle to next non-disabled item, play sound, select new
   * - Other actions
     - Delegate to the currently selected child widget


Free (Cleanup)
^^^^^^^^^^^^^^

Memory cleanup propagates bottom-up through :c:func:`gui_frame_free`:

1. Frame calls :c:func:`component_free` on the root
2. Component unfocuses if focused, then invokes ``free`` callback
3. Sizers iterate children calling :c:func:`component_free` on each, then free local data
4. Widgets call their specific free callback, then free widget data


Widget ID Lookup
^^^^^^^^^^^^^^^^

:c:func:`gui_frame_find` searches the component tree for a widget by ID:

1. Calls :c:func:`component_find` on the root
2. Sizers iterate children, calling :c:func:`component_find` on each
3. Widgets check if their ID matches, return self if so
4. If not found in children, sizers may call specialization's find callback


Layout Algorithm
----------------

The layout phase assigns concrete positions and sizes to all components.

**Menu Layout** (in ``menu_layout``):

.. list-table::
   :widths: 10 90

   * - 1
     - Calculate available space (width for horizontal, height for vertical)
   * - 2
     - Calculate reserved space by summing children with size hints
   * - 3
     - Calculate space per unhinted item from remaining space
   * - 4
     - Select first non-disabled item
   * - 5
     - Calculate centering offset if all items have hints
   * - 6
     - Iterate children: determine size from hint or calculated, call :c:func:`component_layout`, advance offset
   * - 7
     - Create background surfaces if enabled

**XYSizer Layout** (in ``xysizer_layout``):

For each child, reads ``x_hint``, ``y_hint``, ``w_hint``, ``h_hint`` and calls
:c:func:`component_layout` with those values directly. Children must have hints set.


State Management
----------------

Component State Flags
^^^^^^^^^^^^^^^^^^^^^

Each component maintains three state flags, each with a corresponding support flag:

.. list-table::
   :header-rows: 1
   :widths: 25 25 50

   * - State
     - Support Flag
     - Description
   * - ``is_focused``
     - ``supports_focus``
     - Component has keyboard/input focus
   * - ``is_selected``
     - ``supports_select``
     - Component is currently highlighted in menu
   * - ``is_disabled``
     - ``supports_disable``
     - Component is grayed out and non-interactive

**State Setters:**

These functions check the corresponding ``supports_*`` flag before modifying state:

- :c:func:`component_disable` - Set disabled state
- :c:func:`component_select` - Set selected state
- :c:func:`component_focus` - Set focus state (also invokes focus callback)

**State Queries:**

- :c:func:`component_is_disabled` - True if disabled AND supports_disable
- :c:func:`component_is_selected` - True if selected AND supports_select
- :c:func:`component_is_focused` - True if focused AND supports_focus
- :c:func:`component_is_selectable` - Returns supports_select flag


Submenu Management
------------------

Menus support nested submenus with automatic delegation.

**Setting a Submenu:**

Call :c:func:`menu_set_submenu` with the submenu component. The menu will:

1. Initialize the submenu via :c:func:`component_init`
2. Layout the submenu via :c:func:`component_layout`

**During Operation:**

When a submenu is active and not finished:

- ``menu_tick`` only ticks the submenu
- ``menu_render`` only renders the submenu
- ``menu_action`` delegates all actions to the submenu

**Submenu Completion:**

When the submenu's ``finished`` flag becomes true:

- The parent menu's ``submenu_done`` callback is invoked (set via :c:func:`menu_set_submenu_done_cb`)
- The callback receives both the parent menu and submenu for result processing
