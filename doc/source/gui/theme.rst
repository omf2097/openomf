Theme System
============

The theme system provides consistent styling across all GUI components. Themes
define colors, fonts, and visual properties that are propagated through the
component tree during initialization.

.. seealso:: :doc:`/api/game/gui/theme` for the API reference.


Overview
--------

A theme defines:

- **Font**: Size/type for text rendering
- **Colors**: Primary, secondary, active, inactive, disabled, and shadow colors
- **Dialog styling**: Border colors for dialogs and buttons

Themes are copied into frames, not stored by reference.


Theme Propagation
-----------------

The theme is propagated through the component tree during initialization via
:c:func:`gui_frame_layout`:

1. :c:func:`gui_frame_layout` calls :c:func:`component_init` on the root
2. :c:func:`component_init` stores the theme pointer via :c:func:`component_set_theme`
3. :c:func:`component_init` invokes the component's ``init`` callback
4. For sizers, the ``init`` callback calls :c:func:`component_init` on each child

This ensures every component in the tree has access to the theme via
:c:func:`component_get_theme`.


State-Based Color Selection
---------------------------

Widgets automatically select the appropriate color based on their state:

.. code-block:: c

   // Typical pattern in widget render functions
   const gui_theme *theme = component_get_theme(c);

   if(component_is_selected(c)) {
       text_set_color(text, theme->text.active_color);
   } else if(component_is_disabled(c)) {
       text_set_color(text, theme->text.disabled_color);
   } else {
       text_set_color(text, theme->text.inactive_color);
   }

This pattern is used consistently across :c:func:`button_create`, :c:func:`textselector_create`,
:c:func:`textslider_create`, :c:func:`textinput_create`, and other interactive widgets.


Common Color Indices
--------------------

OpenOMF uses VGA palette indices for colors. Common text colors:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Name
     - Value
     - Usage
   * - ``TEXT_DARK_GREEN``
     - 0xA0
     - Disabled items, shadows
   * - ``TEXT_MEDIUM_GREEN``
     - 0xA5
     - Inactive items, primary text
   * - ``TEXT_BRIGHT_GREEN``
     - 0xA7
     - Active/selected items, highlights
   * - ``COLOR_LIGHT_BLUE``
     - Various
     - Help text in menus


Example Themes
--------------

**Main Menu Theme:**

.. code-block:: c

   gui_theme menu_theme;
   gui_theme_defaults(&menu_theme);
   menu_theme.text.font = FONT_BIG;
   menu_theme.text.primary_color = TEXT_MEDIUM_GREEN;
   menu_theme.text.secondary_color = TEXT_BRIGHT_GREEN;
   menu_theme.text.active_color = TEXT_BRIGHT_GREEN;
   menu_theme.text.inactive_color = TEXT_MEDIUM_GREEN;
   menu_theme.text.disabled_color = TEXT_DARK_GREEN;
   menu_theme.dialog.border_color = TEXT_MEDIUM_GREEN;


**Dialog Theme:**

.. code-block:: c

   gui_theme dialog_theme;
   gui_theme_defaults(&dialog_theme);
   dialog_theme.text.font = FONT_BIG;
   dialog_theme.text.primary_color = TEXT_MEDIUM_GREEN;
   dialog_theme.text.secondary_color = TEXT_BRIGHT_GREEN;
   dialog_theme.text.active_color = TEXT_BRIGHT_GREEN;
   dialog_theme.text.inactive_color = TEXT_MEDIUM_GREEN;
   dialog_theme.dialog.border_color = TEXT_MEDIUM_GREEN;


Per-Widget Overrides
--------------------

Many widgets allow overriding theme colors on a per-widget basis:

.. code-block:: c

   // Label with custom color
   component *label = label_create("Custom Color");
   label_set_text_color(label, 0xFF);  // Override theme color

   // Button with shadow
   component *btn = button_create("Shadowed", NULL, false, true, cb, NULL);
   button_set_text_shadow(btn, GLYPH_SHADOW_RIGHT, 0xA0);

These overrides take precedence over theme colors when rendering.
