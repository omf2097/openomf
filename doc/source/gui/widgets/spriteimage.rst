SpriteImage
===========

A simple widget that displays a static sprite image. Unlike SpriteButton,
this widget has no interactivity.

Source: ``src/game/gui/spriteimage.c``

.. seealso:: :doc:`/api/game/gui/spriteimage` for the API reference.


Features
--------

- Displays a single surface
- Optional ownership of the sprite (auto-free)
- No user interaction
- Lightweight implementation


State Support
-------------

SpriteImage is display-only with minimal state:

- ``supports_disable`` = true (but no visual difference)
- ``supports_select`` = false
- ``supports_focus`` = false

The widget is created in disabled state by default.


Call Chains
-----------

**Free** (``spriteimage_free``):

1. Check if widget owns the sprite
2. If ownership is set:

   - Free surface memory via :c:func:`surface_free`

3. Free local widget data


Ownership
---------

By default, the widget does NOT own the surface. The caller is responsible
for keeping the surface alive and freeing it.

If you want the widget to take ownership:

.. code-block:: c

   surface *img = omf_malloc(sizeof(surface));
   surface_create_from_data(img, w, h, data);

   component *si = spriteimage_create(img);
   spriteimage_set_owns_sprite(si, true);  // Widget will free img

.. warning::
   When ownership is transferred, the widget casts away const to free the
   surface. Ensure you pass a heap-allocated surface.


Usage Examples
--------------

**Basic Usage (External Ownership):**

.. code-block:: c

   // Surface must outlive the widget
   static surface my_image;
   surface_create_from_data(&my_image, 32, 32, pixel_data);

   component *img = spriteimage_create(&my_image);

**With Widget Ownership:**

.. code-block:: c

   surface *dynamic_img = omf_malloc(sizeof(surface));
   surface_create_from_data(dynamic_img, 32, 32, data);

   component *img = spriteimage_create(dynamic_img);
   spriteimage_set_owns_sprite(img, true);
   // dynamic_img will be freed when widget is freed

**In a Layout:**

.. code-block:: c

   component *sizer = xysizer_create();

   component *logo = spriteimage_create(&logo_surface);
   xysizer_attach(sizer, logo, 100, 50, logo_surface.w, logo_surface.h);


Comparison with SpriteButton
----------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Feature
     - SpriteImage
     - SpriteButton
   * - Interactivity
     - None
     - Click, Focus callbacks
   * - Text overlay
     - No
     - Yes
   * - Visual feedback
     - None
     - Active ticks animation
   * - Disabled visual
     - None
     - Gray tint
   * - Use case
     - Static decoration
     - Interactive buttons
