GUI Frame
=========

The :c:struct:`gui_frame` structure is the top-level container that owns and manages
a component tree. It holds the theme and coordinates initialization, layout,
rendering, and event handling.

Source: ``src/game/gui/gui_frame.c``

.. seealso:: :doc:`/api/game/gui/gui_frame` for the API reference.


Overview
--------

The GUI frame serves as:

- **Owner** of the component tree (frees all components when destroyed)
- **Theme holder** (stores a copy of the theme, passes it during init)
- **Coordinator** for initialization and layout (two-phase setup)
- **Dispatcher** for events and actions to the component tree
- **Entry point** for the game loop (tick, render)

.. note::

   The :c:struct:`gui_frame` is an **opaque type** - its structure is defined
   in the ``.c`` file, not the header. You can only interact with it through
   the public API functions.


Architecture
------------

The frame owns a theme (copied during creation) and a root component that forms
the root of the component tree.

**Ownership Model:**

- :c:struct:`gui_frame` contains a copy of :c:struct:`gui_theme`
- Frame owns the root component pointer
- Root component owns the entire component tree
- When the frame is freed via :c:func:`gui_frame_free`, the entire tree is freed


Two-Phase Layout
----------------

The frame performs two-phase setup via :c:func:`gui_frame_layout`:

**Phase 1 - Initialization** (via :c:func:`component_init`):

- Propagates theme to all components
- Widgets pre-render text and calculate size hints
- Callbacks receive the theme for font/color configuration

**Phase 2 - Layout** (via :c:func:`component_layout`):

- Assigns concrete positions to all components
- Assigns sizes based on hints and available space
- Sizers create background surfaces if configured

Both phases are required before the frame can be used in the game loop.


Lifecycle
---------

.. graphviz::
   :caption: GUI Frame Lifecycle States

   digraph lifecycle {
      rankdir=LR
      node [shape=box, style="rounded,filled", fillcolor=lightblue]

      created [label="Created"]
      has_root [label="Has Root"]
      laid_out [label="Laid Out"]
      running [label="Running", fillcolor=lightgreen]
      freed [label="Freed", fillcolor=lightgray]

      created -> has_root [label="set_root()"]
      has_root -> laid_out [label="layout()"]
      laid_out -> running [label="game loop"]
      running -> running [label="tick/render/\nevent/action"]
      running -> has_root [label="set_root()\n(replaces tree)"]

      created -> freed [style=dashed]
      has_root -> freed [style=dashed]
      laid_out -> freed [style=dashed]
      running -> freed [label="free()"]
   }

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - State
     - Description
   * - Created
     - After :c:func:`gui_frame_create`. Has theme, no root component.
   * - Has Root
     - After :c:func:`gui_frame_set_root`. Tree is built but not initialized.
   * - Laid Out
     - After :c:func:`gui_frame_layout`. Ready for game loop.
   * - Running
     - During game loop. Call tick, render, event, action as needed.
   * - Freed
     - After :c:func:`gui_frame_free`. Frame and all components destroyed.

**Transitions:**

- ``Created → Has Root``: Call :c:func:`gui_frame_set_root`
- ``Has Root → Laid Out``: Call :c:func:`gui_frame_layout`
- ``Running → Has Root``: Call :c:func:`gui_frame_set_root` with new tree
- ``Any → Freed``: Call :c:func:`gui_frame_free`


Usage Example
-------------

**Basic Setup:**

.. code-block:: c

   // Set up theme
   gui_theme theme;
   gui_theme_defaults(&theme);
   theme.text.primary_color = TEXT_MEDIUM_GREEN;
   theme.text.active_color = TEXT_BRIGHT_GREEN;

   // Create frame with position and size
   gui_frame *frame = gui_frame_create(&theme, 10, 10, 300, 200);

   // Build component tree
   component *menu = menu_create();
   menu_attach(menu, button_create("Start", "Begin the game",
                                    false, false, start_cb, NULL));
   menu_attach(menu, button_create("Options", "Configure settings",
                                    false, false, options_cb, NULL));
   menu_attach(menu, button_create("Quit", "Exit to desktop",
                                    false, false, quit_cb, NULL));

   // Set root and perform two-phase initialization
   gui_frame_set_root(frame, menu);
   gui_frame_layout(frame);  // REQUIRED before tick/render

   // Game loop
   while(running) {
       // Handle SDL events (for text input, etc.)
       SDL_Event event;
       while(SDL_PollEvent(&event)) {
           if(gui_frame_event(frame, &event) == 0) {
               continue;  // Event was handled by GUI
           }
           // Handle other events...
       }

       // Handle controller actions (menu navigation)
       // Actions are obtained via controller_poll() in the actual game
       int action = /* get action from controller */;
       if(action != ACT_NONE) {
           gui_frame_action(frame, action);
       }

       // Update animations
       gui_frame_tick(frame);

       // Render GUI
       gui_frame_render(frame);
   }

   // Cleanup - frees frame AND entire component tree
   gui_frame_free(frame);


Integration with Scenes
-----------------------

In OpenOMF, each scene typically owns its own :c:struct:`gui_frame`:

.. code-block:: c

   typedef struct my_scene {
       gui_frame *frame;
       // ... other scene data ...
   } my_scene;

   void my_scene_create(my_scene *scene) {
       gui_theme theme;
       gui_theme_defaults(&theme);

       scene->frame = gui_frame_create(&theme, 0, 0, NATIVE_W, NATIVE_H);
       gui_frame_set_root(scene->frame, create_scene_menu());
       gui_frame_layout(scene->frame);
   }

   void my_scene_tick(my_scene *scene) {
       gui_frame_tick(scene->frame);
   }

   void my_scene_render(my_scene *scene) {
       // Render scene background...
       gui_frame_render(scene->frame);
   }

   int my_scene_action(my_scene *scene, int action) {
       return gui_frame_action(scene->frame, action);
   }

   void my_scene_free(my_scene *scene) {
       gui_frame_free(scene->frame);
   }


Dynamic Tree Replacement
------------------------

You can replace the entire component tree at runtime:

.. code-block:: c

   void switch_to_options_menu(gui_frame *frame) {
       // Create new tree
       component *options = create_options_menu();

       // Replace root - old tree is automatically freed
       gui_frame_set_root(frame, options);

       // MUST re-layout after changing root
       gui_frame_layout(frame);
   }

.. warning::

   After calling :c:func:`gui_frame_set_root`, any pointers to components
   from the old tree are **invalid** and must not be used.


Finding Widgets by ID
---------------------

Widgets can be assigned IDs for later lookup:

.. code-block:: c

   // During tree construction
   component *selector = textselector_create("RESOLUTION", NULL,
                                             on_resolution_change, NULL);
   widget_set_id(selector, WIDGET_ID_RESOLUTION);
   menu_attach(options_menu, selector);

   // Later, to access the widget
   component *res = gui_frame_find(frame, WIDGET_ID_RESOLUTION);
   if(res != NULL) {
       textselector_set_pos(res, new_resolution);
   }
