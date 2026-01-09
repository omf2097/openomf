TextSlider
==========

A visual slider widget with discrete positions displayed as filled/unfilled bars.

Source: ``src/game/gui/textslider.c``

.. seealso:: :doc:`/api/game/gui/textslider` for the API reference.


Features
--------

- Visual bar representation of position
- Optional "OFF" state at position 0
- Slide callback on position change
- Bindable position to external variable
- Configurable number of positions
- Optional audio panning on slide


Display Format
--------------

The slider displays: ``"TITLE ████░░░░"``

- ``█`` (CURSOR_CHAR) = Filled position
- ``|`` = Unfilled position
- If ``has_off`` and ``pos == 0``: Shows ``"TITLE OFF"``


Input Handling
--------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Action
     - Behavior
   * - ``ACT_KICK``
     - Increase position (clamped at max)
   * - ``ACT_PUNCH``
     - Increase position (clamped at max)
   * - ``ACT_RIGHT``
     - Increase position (clamped at max)
   * - ``ACT_LEFT``
     - Decrease position (clamped at 0)

Unlike TextSelector, the slider does NOT wrap - it clamps at min/max.


Call Chains
-----------

**Refresh (Display Update):**

1. Start with title string: ``"TITLE "``
2. Check if ``has_off`` and ``pos == 0``:

   - If true, append ``"OFF"`` and return

3. For each position from 0 to ``positions``:

   - If ``i < pos``, append filled marker (█)
   - Otherwise, append unfilled marker (|)


**Action** (``textslider_action``):

1. Save current position
2. Check action type:

   .. list-table::
      :header-rows: 1
      :widths: 30 70

      * - Action
        - Operation
      * - ``ACT_KICK``, ``ACT_PUNCH``, ``ACT_RIGHT``
        - Increment position (clamp at max)
      * - ``ACT_LEFT``
        - Decrement position (clamp at 0)

3. If position changed:

   - Refresh display text
   - Play slide sound with panning via :c:func:`audio_play_sound`
   - Call slide callback if set
   - Return 0 (handled)

4. If position unchanged, return 1 (not handled)


Audio Panning
-------------

By default, sliding left/right plays sound with corresponding stereo panning:

- Left: panning = -0.5
- Right: panning = 0.5

This can be disabled with :c:func:`textslider_disable_panning`.


Usage Examples
--------------

**Volume Slider (with OFF):**

.. code-block:: c

   void on_volume(component *c, void *userdata, int pos) {
       set_volume(pos / 10.0f);
   }

   component *vol = textslider_create(
       "VOLUME",
       "Adjust game volume",
       10,      // 10 positions
       1,       // has OFF at position 0
       on_volume,
       NULL
   );

**Brightness Slider (no OFF):**

.. code-block:: c

   component *bright = textslider_create(
       "BRIGHTNESS",
       NULL,
       5,       // 5 positions
       0,       // no OFF
       on_brightness,
       NULL
   );

**Bound Slider:**

.. code-block:: c

   int music_volume = 5;

   component *slider = textslider_create_bind(
       "MUSIC",
       "Music volume",
       10, 1,
       on_music_change,
       NULL,
       &music_volume
   );


Visual Example
--------------

With 5 positions and ``has_off = true``:

- Position 0: ``VOLUME OFF``
- Position 1: ``VOLUME █||||``
- Position 3: ``VOLUME ███||``
- Position 5: ``VOLUME █████``
