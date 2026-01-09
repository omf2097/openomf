Sizer Base
==========

The :c:struct:`sizer` structure is the base class for all container components. It manages
a vector of child components and provides callback routing.

Source: ``src/game/gui/sizer.c``

.. seealso:: :doc:`/api/game/gui/sizer` for the API reference.


Overview
--------

The sizer layer provides:

- **Child management** - Vector of child components
- **Callback routing** - Routes component callbacks to sizer-specific implementations
- **Specialization pointer** - Points to sizer-specific data (menu, xysizer, etc.)
- **Opacity control** - For fade effects

All sizers use the magic header ``0xDEADBEEF`` (``SIZER_MAGIC``) for runtime type checking.


Call Chains
-----------

**Tick** (``sizer_tick``):

1. If specialization tick callback is set, call it
2. Iterate all children, calling :c:func:`component_tick` on each

**Init** (``sizer_init``):

1. If specialization init callback is set, call it
2. Iterate all children, calling :c:func:`component_init` on each

**Free** (``sizer_free``):

1. Iterate all children, calling :c:func:`component_free` on each
2. If specialization free callback is set, call it
3. Free the children vector
4. Free the sizer data structure

**Find** (``sizer_find``):

1. Iterate all children, calling :c:func:`component_find` on each
2. If a child returns non-NULL, return that result immediately
3. After all children checked, if specialization find callback is set, call it
4. Otherwise return NULL


Usage Example
-------------

Creating a custom sizer:

.. code-block:: c

   // Custom sizer structure
   typedef struct my_sizer {
       int custom_data;
   } my_sizer;

   static void my_sizer_render(component *c) {
       // Render background
       // ...

       // Render children
       iterator it;
       component **tmp;
       sizer_begin_iterator(c, &it);
       foreach(it, tmp) {
           component_render(*tmp);
       }
   }

   static void my_sizer_layout(component *c, int x, int y, int w, int h) {
       // Position children
       iterator it;
       component **tmp;
       sizer_begin_iterator(c, &it);
       int offset = 0;
       foreach(it, tmp) {
           component_layout(*tmp, x, y + offset, w, 20);
           offset += 25;
       }
   }

   static void my_sizer_free(component *c) {
       my_sizer *local = sizer_get_obj(c);
       omf_free(local);
   }

   component *my_sizer_create(void) {
       component *c = sizer_create();

       my_sizer *local = omf_calloc(1, sizeof(my_sizer));
       sizer_set_obj(c, local);

       sizer_set_render_cb(c, my_sizer_render);
       sizer_set_layout_cb(c, my_sizer_layout);
       sizer_set_free_cb(c, my_sizer_free);

       return c;
   }


Iteration Pattern
-----------------

.. code-block:: c

   iterator it;
   component **tmp;
   sizer_begin_iterator(container, &it);
   foreach(it, tmp) {
       component *child = *tmp;
       // Process child
   }
