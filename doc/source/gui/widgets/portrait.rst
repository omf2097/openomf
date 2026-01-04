Portrait
========

A specialized widget for displaying fighter/pilot portraits loaded from
PIC resource files.

Source: ``src/game/gui/portrait.c``

.. seealso:: :doc:`/api/game/gui/portrait` for the API reference.


Features
--------

- Loads portraits from PIC files (game resources)
- Supports multiple pilots per PIC
- Next/previous navigation
- Can be set from raw sprite data
- Automatic palette loading


State Support
-------------

Portrait is display-only:

- ``supports_disable`` = false
- ``supports_select`` = false
- ``supports_focus`` = false


Call Chains
-----------

**Navigation** (:c:func:`portrait_next` / :c:func:`portrait_prev`):

**portrait_next:**

1. Increment selection index
2. If at max, wrap to first (index 0)
3. Select new portrait via internal selection function

**portrait_prev:**

1. Decrement selection index
2. If below zero, wrap to last
3. Select new portrait via internal selection function


Usage Examples
--------------

**Basic Portrait:**

.. code-block:: c

   // Load pilot 0 from PICS4.PIC
   component *portrait = portrait_create(PIC_PILOT1, 0);

**Navigation:**

.. code-block:: c

   // Handle left/right to cycle pilots
   void on_action(component *c, int action) {
       if(action == ACT_LEFT) {
           portrait_prev(portrait);
       } else if(action == ACT_RIGHT) {
           portrait_next(portrait);
       }
   }

**Query Current Selection:**

.. code-block:: c

   int current = portrait_selected(portrait);
   printf("Selected pilot: %d\n", current);

**Direct Sprite Assignment:**

.. code-block:: c

   sd_sprite my_sprite;
   // ... load sprite data ...
   portrait_set_from_sprite(portrait, &my_sprite);

**Using the Load Helper:**

.. code-block:: c

   sd_sprite spr;
   vga_palette pal;
   sd_sprite_create(&spr);

   if(portrait_load(&spr, &pal, PIC_PILOT1, 0) == SD_SUCCESS) {
       // Use spr and pal
   }

   sd_sprite_free(&spr);


Resource IDs
------------

Common PIC resource IDs for portraits:

- ``PIC_PILOT1``, ``PIC_PILOT2``, etc. - Pilot portraits
- Various HAR-specific portrait files

The exact IDs are defined in ``resources/ids.h``.
