Dialog
======

A high-level component for displaying modal dialogs with pre-configured buttons.
Unlike other containers, Dialog wraps a :c:struct:`gui_frame` rather than being a raw sizer.

Source: ``src/game/gui/dialog.c``

.. seealso:: :doc:`/api/game/gui/dialog` for the API reference.


Features
--------

- Pre-built dialog layouts (Yes/No, OK, Cancel)
- Customizable text message
- Built-in theme with green text
- Show/hide visibility control
- Callback for button clicks
- ESC key handling


Component Structure
-------------------

The dialog creates a component tree internally:

- **gui_frame** (root owner)

  - **Menu** (vertical, contains all elements)

    - **Label** (title text)
    - **Filler** (spacing)
    - **Menu** (horizontal, contains buttons)

      - **Button** (YES/OK/CANCEL - always present)
      - **Button** (NO - only for YES_NO style)


Theme
-----

Dialogs use a custom green theme:

.. code-block:: c

   gui_theme theme;
   gui_theme_defaults(&theme);
   theme.dialog.border_color = TEXT_MEDIUM_GREEN;
   theme.text.font = FONT_BIG;
   theme.text.primary_color = TEXT_MEDIUM_GREEN;
   theme.text.secondary_color = TEXT_BRIGHT_GREEN;
   theme.text.active_color = TEXT_BRIGHT_GREEN;
   theme.text.inactive_color = TEXT_MEDIUM_GREEN;


Call Flow
---------

**Creation** (:c:func:`dialog_create`):

1. Initialize dialog fields and create theme
2. Create vertical menu for overall layout
3. Add title label with text
4. Add filler for spacing
5. Create horizontal button menu
6. Add buttons based on style (CANCEL, YES_NO, or OK)
7. Create :c:struct:`gui_frame` with theme
8. Set menu as root and call :c:func:`gui_frame_layout`

**Event Handling** (:c:func:`dialog_event`):

1. If not visible, return immediately
2. Delegate action to frame via :c:func:`gui_frame_action`
3. If ESC was pressed and callback is set, call callback with cancel result


Usage Examples
--------------

**Confirmation Dialog:**

.. code-block:: c

   dialog confirm;

   void on_dialog(dialog *dlg, dialog_result result) {
       if(result == DIALOG_RESULT_YES_OK) {
           do_the_thing();
       }
       dialog_show(dlg, 0);
   }

   void show_confirm() {
       dialog_create(&confirm, DIALOG_STYLE_YES_NO,
                     "Are you sure?", 50, 70);
       confirm.clicked = on_dialog;
       dialog_show(&confirm, 1);
   }

   // In game loop:
   if(dialog_is_visible(&confirm)) {
       dialog_tick(&confirm);
       dialog_render(&confirm);
   }

   // Handle input
   dialog_event(&confirm, action);

   // Cleanup
   dialog_free(&confirm);


**OK Dialog:**

.. code-block:: c

   dialog alert;
   dialog_create(&alert, DIALOG_STYLE_OK,
                 "Operation completed!", 50, 70);
   alert.clicked = dismiss_dialog;
   dialog_show(&alert, 1);


**Cancel Dialog:**

.. code-block:: c

   dialog loading;
   dialog_create(&loading, DIALOG_STYLE_CANCEL,
                 "Loading...", 50, 70);
   loading.clicked = cancel_loading;
   dialog_show(&loading, 1);


**Custom Height:**

.. code-block:: c

   dialog big;
   dialog_create_h(&big, DIALOG_STYLE_OK,
                   "This is a much longer message that needs more space",
                   30, 50, 100);  // Height = 100


Integration Pattern
-------------------

Typical integration in a scene:

.. code-block:: c

   typedef struct my_scene {
       dialog confirm_dialog;
       bool dialog_active;
   } my_scene;

   void scene_create(my_scene *s) {
       dialog_create(&s->confirm_dialog, DIALOG_STYLE_YES_NO,
                     "Quit game?", 50, 80);
       s->confirm_dialog.clicked = on_confirm;
       s->confirm_dialog.userdata = s;
   }

   void scene_action(my_scene *s, int action) {
       if(s->dialog_active) {
           dialog_event(&s->confirm_dialog, action);
           return;
       }
       // Normal scene handling...
   }

   void scene_render(my_scene *s) {
       // Render scene...

       if(s->dialog_active) {
           dialog_render(&s->confirm_dialog);
       }
   }

   void scene_free(my_scene *s) {
       dialog_free(&s->confirm_dialog);
   }
