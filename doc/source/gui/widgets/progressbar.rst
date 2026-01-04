ProgressBar
===========

An animated progress bar widget with smooth transitions, flashing effects,
and theme-based styling. Used for health bars, endurance meters, and similar displays.

Source: ``src/game/gui/progressbar.c``

.. seealso:: :doc:`/api/game/gui/progressbar` for the API reference.


Features
--------

- Smooth animated transitions (1% per tick)
- Multiple color themes (health, endurance, melee)
- Flashing effect with configurable rate
- Left or right orientation
- Highlight mode for visual emphasis
- Pre-rendered background and block surfaces


State Support
-------------

ProgressBar is display-only:

- ``supports_disable`` = false
- ``supports_select`` = false
- ``supports_focus`` = false


Animation System
----------------

The progress bar animates smoothly when decreasing:

1. :c:func:`progressbar_set_progress` called with new percentage
2. Check animation mode:

   - If ``animate = false`` or value is increasing: set display to target immediately
   - If ``animate = true`` and decreasing: set target, keep current display

3. During render, if ``display > percentage``:

   - Decrease display value toward target (1% per frame)
   - Regenerate bar surface


Call Chains
-----------

**Tick** (``progressbar_tick``):

1. Check if flashing is enabled
2. If flashing and tick counter exceeds rate:

   - Reset tick counter
   - Toggle flash state (visible/hidden)

3. Increment tick counter


**Render** (``progressbar_render``):

1. Check if refresh needed or display > percentage:

   - Clear refresh flag
   - If display > percentage, decrement display (animate down)
   - Free old block surface
   - Calculate bar width from current percentage
   - If valid size, create new block surface

2. Draw background surface
3. Draw progress block surface


Usage Examples
--------------

**Health Bar:**

.. code-block:: c

   component *health = progressbar_create(
       PROGRESSBAR_THEME_HEALTH,
       PROGRESSBAR_LEFT,
       100  // Start at 100%
   );
   component_set_size_hints(health, 100, 8);

**Endurance Bar (Right-oriented):**

.. code-block:: c

   component *endurance = progressbar_create(
       PROGRESSBAR_THEME_ENDURANCE,
       PROGRESSBAR_RIGHT,
       75
   );

**Taking Damage (Animated):**

.. code-block:: c

   // Smoothly animate from current to new value
   progressbar_set_progress(health, 60, true);

**Healing (Instant):**

.. code-block:: c

   // Instantly jump to new value
   progressbar_set_progress(health, 80, false);

**Low Health Flashing:**

.. code-block:: c

   // Enable flashing every 10 ticks
   if(health_percent < 20) {
       progressbar_set_flashing(health, 1, 10);
   } else {
       progressbar_set_flashing(health, 0, 0);
   }

**Highlight Mode:**

.. code-block:: c

   progressbar_set_highlight(bar, true);
