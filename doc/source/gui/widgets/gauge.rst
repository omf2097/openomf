Gauge
=====

A visual indicator widget that displays a row of lit and unlit segments,
typically used for stats like power, speed, or agility.

Source: ``src/game/gui/gauge.c``

.. seealso:: :doc:`/api/game/gui/gauge` for the API reference.


Features
--------

- Two visual styles: small (3x3 px) and big (8x3 px) segments
- Configurable total size and lit count
- Pre-rendered pixel graphics
- No user interaction


State Support
-------------

Gauge is display-only:

- ``supports_disable`` = false
- ``supports_select`` = false
- ``supports_focus`` = false


Pixel Graphics
--------------

The gauge uses pre-defined pixel images for segments:

**Small Gauge (3x3 pixels):**

.. code-block:: none

   OFF:              ON:
   ▓▓░              ██▓
   ▓░░              ▓▓▓
   ░░░              ▓▓░

**Big Gauge (8x3 pixels):**

.. code-block:: none

   OFF:                      ON:
   ▓▓░░░░░░                  ████████▓
   ▓░░░░░░░                  ▓▓▓▓▓▓▓▓
   ░░░░░░░░                  ▓▓▓▓▓▓▓░

The colors use VGA palette indices 0xA0-0xA7 (green gradient).


Call Chains
-----------

**Creation** (:c:func:`gauge_create`):

1. Create base widget via :c:func:`widget_create`
2. Disable all state support via :c:func:`component_set_supports`
3. Allocate gauge struct
4. Set size, type, and lit count (clamped to size)
5. Load segment images based on type:

   - Small type: load 3x3 ON and OFF images
   - Big type: load 8x3 ON and OFF images


**Render** (``gauge_render``):

1. Initialize x position to ``c->x``
2. For each segment from 0 to ``lit``:

   - Draw ON segment image
   - Advance x by segment width

3. For each segment from ``lit`` to ``size``:

   - Draw OFF segment image
   - Advance x by segment width


Usage Examples
--------------

**Stat Gauge (Small):**

.. code-block:: c

   // 10 segments, 7 lit
   component *power = gauge_create(GAUGE_SMALL, 10, 7);

**Progress Gauge (Big):**

.. code-block:: c

   // 5 segments, 3 lit
   component *progress = gauge_create(GAUGE_BIG, 5, 3);

**Dynamic Update:**

.. code-block:: c

   // Update lit segments
   gauge_set_lit(gauge, new_value);

   // Resize gauge
   gauge_set_size(gauge, new_size);
   if(gauge_get_lit(gauge) > new_size) {
       // lit is automatically clamped
   }

**Query State:**

.. code-block:: c

   int current = gauge_get_lit(gauge);
   int max = gauge_get_size(gauge);
   printf("Progress: %d/%d\n", current, max);


Size Hints
----------

The gauge automatically sets size hints based on segment dimensions:

- Small: ``(3 * size, 3)`` pixels
- Big: ``(8 * size, 3)`` pixels
