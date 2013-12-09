/* keyresponses.c
 * for Denemo, a gtk+ frontend to GNU Lilypond
 * (c) 1999-2005 Matthew Hiller
 */

#include <string.h>
#include "kbd-custom.h"
#include "keyresponses.h"
#include "articulations.h"
#include "barline.h"
#include "utils.h"
#include "tupletops.h"
#include "view.h"
#include "commandfuncs.h"
#include "kbd-custom.h"
#include "audiointerface.h"
#if GTK_MAJOR_VERSION==3
#include <gdk/gdkkeysyms-compat.h>      //FIXME Look for something more gtk3 like
#endif
static GdkEventKey **divert_key_event;  /* Non null if key events are being intercepted by a function
                                         * (which is running a gtk_mail_loop() for this reason).
                                         * return TRUE if a key press successfully captured
                                         * in which case the kyval, state pair are returned
                                         */

static gint divert_key_id = 0;
gboolean
intercept_scorearea_keypress (GdkEventKey * pevent)
{
  if (divert_key_event)
    {
      warningdialog (_("Recursive key capture not possible!"));    /* we could make a stack of them instead ... */
      return FALSE;
    }
  GdkEventKey *event;
  divert_key_id = Denemo.project->id;
  divert_key_event = &event;
  gtk_main ();
  divert_key_event = NULL;
  // *keyval = event->keyval;
  // *state = dnm_sanitize_key_state(event);
  memcpy (pevent, event, sizeof (GdkEventKey));
  return TRUE;
}



static guint
lock_mask (gint keyval)
{

  if ((keyval == GDK_Shift_L) || (keyval == GDK_Shift_R))
    return GDK_SHIFT_MASK;

  if (keyval == GDK_Caps_Lock)
    return GDK_LOCK_MASK;

  if ((keyval == GDK_Control_L) || (keyval == GDK_Control_R))
    return GDK_CONTROL_MASK;


  if (keyval == GDK_Alt_L)
    return GDK_MOD1_MASK;



  if (keyval == GDK_Num_Lock)
    return GDK_MOD2_MASK;

  /*penguin/windows */
  if ((keyval == GDK_Super_L) || (keyval == GDK_Super_R))
    return GDK_MOD4_MASK;

  if (keyval == GDK_Alt_R || GDK_ISO_Level3_Shift)
    return GDK_MOD5_MASK;

  return 0;

}

static guint
klock_mask (gint keyval)
{

  if ((keyval == GDK_Shift_L) || (keyval == GDK_Shift_R))
    return GDK_SHIFT_MASK;

  if ((keyval == GDK_Control_L) || (keyval == GDK_Control_R))
    return GDK_CONTROL_MASK;


  if (keyval == GDK_Alt_L)
    return GDK_MOD1_MASK;


  /*penguin/windows */
  if ((keyval == GDK_Super_L) || (keyval == GDK_Super_R))
    return GDK_MOD4_MASK;

  if (keyval == GDK_Alt_R || GDK_ISO_Level3_Shift)
    return GDK_MOD5_MASK;

  return 0;

}

static guint
llock_mask (gint keyval)
{
  if (keyval == GDK_Caps_Lock)
    return GDK_LOCK_MASK;


  if (keyval == GDK_Num_Lock)
    return GDK_MOD2_MASK;

  return 0;

}


/**
 * keyrelease event callback 
 * sets cursor if a modifier
 */

gint
scorearea_keyrelease_event (GtkWidget * widget, GdkEventKey * event)
{
  Denemo.keyboard_state ^= (0xf & klock_mask (event->keyval));
  if ((event->keyval == GDK_Alt_L) || (event->keyval == GDK_Alt_R))
    {
      if ((Denemo.keyboard_state & CHORD_MASK)) //At least one note has been entered in a chord
        next_editable_note ();
      Denemo.keyboard_state &= ~CHORD_MASK;
    }
  set_midi_in_status ();
  //g_debug("release %x state %x\n", Denemo.keyboard_state, event->state);
  // set_cursor_for(keyrelease_modify(event->state), event->keyval);
  gint state;
  if ((event->keyval == GDK_Caps_Lock) || (event->keyval == GDK_Num_Lock))
    return TRUE;
  state = (lock_mask (event->keyval) ^ event->state);

  set_cursor_for (state);
  return TRUE;
}

/* perform the command of the given name and store the event that triggered it */
static gchar *
perform_command (const gchar * command_name, GdkEventKey * event)
{
  Denemo.last_keyval = event->keyval;
  Denemo.last_keystate = dnm_sanitize_key_state (event);
  call_out_to_guile ("(define DenemoKeypressActivatedCommand #t)");
  execute_callback_from_name (command_name);
  call_out_to_guile ("(define DenemoKeypressActivatedCommand #f)");
  // note Denemo.project = Denemo.project; may have changed as a result of executing the command
#ifdef TESTING_REPEATED_XPOSITION_UPDATE
  if (Denemo.project->si)
    displayhelper (Denemo.project);
#endif
  return NULL;
}

//return the value of perform_command if executed or "" if keypress is part of a two-key shortcut, or NULL toherwise
gchar *
process_key_event (GdkEventKey * event, gchar * perform_command ())
{
  keymap *the_keymap = Denemo.map;
  //g_debug("\n********\nCaps Lock %x?\n\n********\nShifted %x?\n", event->state&GDK_LOCK_MASK,          event->state&GDK_SHIFT_MASK     );
  {
    gint state;
    state = (lock_mask (event->keyval) ^ event->state);
    if (state || ((event->keyval == GDK_Caps_Lock) || (event->keyval == GDK_Num_Lock)))
      set_cursor_for (state);   // MUST LOOK AHEAD to state after keypress HERE CATCH modifiers and set the cursor for them.....
  }
  dnm_clean_event (event);


  if (isModifier (event))
    return NULL;

  /* Look up the keystroke in the keymap and execute the appropriate
   * function */
  static GString *prefix_store = NULL;
  if (!prefix_store)
    prefix_store = g_string_new ("");

  gint command_idx = lookup_command_for_keyevent (event);
  if ((prefix_store->len == 0) && (command_idx != -1))
    {
      const gchar *command_name = lookup_name_from_idx (the_keymap, command_idx);
      if (command_name)
        {
          if (Denemo.prefs.learning)
            {
              gchar *name = dnm_accelerator_name (event->keyval, event->state);
              KeyStrokeShow (name, command_idx, TRUE);
              g_free (name);
            }
          if (Denemo.ScriptRecording)
						if (idx_has_callback (the_keymap, command_idx))
							{
								append_scheme_call ((gchar *) command_name);
							}  
          //g_debug("Single Key shortcut %s invokes %s\n", dnm_accelerator_name(event->keyval, event->state), command_name);
          return perform_command (command_name, event);
        }
      else
        {
          g_warning ("Error: action %i has no name", command_idx);
          return NULL;
        }
    }

  /*  we create a store for the prefix char and look to see if it is populated when a keystroke is received. If it is populated, we try for the two-key combination, {???else we try for the single key, and if that fails populate the store. OR if it fails clear store}. If the two-key combination works we clear the store. If the two-key combination fails we try for the single key, if that succeeds we clear the store if it fails we set the store to the unresolved keystroke.  */

  gchar *ret = NULL;
  if (prefix_store->len)
    {
      gchar *name = dnm_accelerator_name (event->keyval, event->state);
      //g_debug("second key %s\n", name);
      g_string_append_printf (prefix_store, "%c%s", ',', name);
      command_idx = lookup_command_for_keybinding_name (Denemo.map, prefix_store->str);

      if (command_idx != -1)
        {
          const gchar *command_name = lookup_name_from_idx (the_keymap, command_idx);
          if (command_name)
            {
              if (Denemo.prefs.learning)
                {
                  KeyStrokeShow (prefix_store->str, command_idx, FALSE);
                }
					if (Denemo.ScriptRecording)
							if (idx_has_callback (the_keymap, command_idx))
								{
									append_scheme_call ((gchar *) command_name);
								}            
              ret = perform_command (command_name, event);
            }
        }
      else
        {                       //Two key name was not a binding
          ret = NULL;
          write_status (Denemo.project);
          if ((Denemo.project->view != DENEMO_MENU_VIEW) || Denemo.prefs.learning)
            {
							Denemo.prefs.learning = TRUE;
              KeyStrokeDecline (prefix_store->str);
            }
          toggle_to_drawing_area (TRUE);        //restore menus, in case the user is lost and needs to look up a keypress
          if (Denemo.project->view != DENEMO_MENU_VIEW)
						toggle_to_drawing_area (TRUE);
        }
      g_string_assign (prefix_store, "");
      Denemo.continuations = NULL;
      return ret;
    }
  else
    {                           //no prefix stored 
      gchar *name = dnm_accelerator_name (event->keyval, event->state); //FIXME free name

      if ((Denemo.continuations = (GList *) g_hash_table_lookup (Denemo.map->continuations_table, name)))
        {
          GList *g;
          GString *continuations = g_string_new ("");
          for (g = Denemo.continuations; g; g = g->next)
            g_string_append_printf (continuations, "%s%s", (gchar *) g->data, ", or ");
          g_string_printf (prefix_store, "Prefix Key %s, waiting for key %stype Esc to abort", name, continuations->str);
          g_string_free (continuations, TRUE);
          if (Denemo.prefs.immediateplayback)
              play_note (DEFAULT_BACKEND, 0, 9, 61, 300, 127 * Denemo.project->si->master_volume);
          //gtk_statusbar_pop (GTK_STATUSBAR (Denemo.statusbar), Denemo.status_context_id);
          gtk_label_set_text (GTK_LABEL (Denemo.statuslabel), prefix_store->str);
          g_string_assign (prefix_store, name);
          if (Denemo.prefs.learning)
            {
              KeyStrokeAwait (name);
            }
          return "";            //continuation available
        }
      else
        {
          if ((Denemo.project->view != DENEMO_MENU_VIEW) || Denemo.prefs.learning)
            {
							Denemo.prefs.learning = TRUE;
              KeyStrokeDecline (name);
            }
          toggle_to_drawing_area (TRUE);  //restore menus, in case the user is lost and needs to look up a keypress
          if (Denemo.project->view != DENEMO_MENU_VIEW)
						toggle_to_drawing_area (TRUE);
        }
      return NULL;
    }
  return NULL;
}


/**
 * keypress event callback 
 * looks up the key press and executes the correct function
 */

gint
scorearea_keypress_event (GtkWidget * widget, GdkEventKey * event)
{
  Denemo.keyboard_state |= (0xf & klock_mask (event->keyval));
  Denemo.keyboard_state ^= llock_mask (event->keyval);
  // if((event->keyval==GDK_Alt_L)||(event->keyval==GDK_Alt_R))
  //  Denemo.keyboard_state |= CHORD_MASK;
  set_midi_in_status ();

  //g_debug("press Denemo %x state %x klock %x\n", Denemo.keyboard_state, event->state, klock_mask(event->keyval));

  //g_debug("State eored %x\n", (lock_mask(event->keyval)^event->state));
  if (divert_key_event && !isModifier (event) && divert_key_id == Denemo.project->id)
    {
      dnm_clean_event (event);
      *divert_key_event = event;
      //g_object_ref(event); FIXME do we need to keep it around?
      gtk_main_quit ();
      return TRUE;              //*is* reached main loop exits to the caller of the loop when it next gains control
    }

  (void) process_key_event (event, perform_command);
  return TRUE;                  //I think this means do not run any other handlers after this.
}

/**
 * Reduce the measure width by 10 pixels
 */
void
adjust_measure_less_width_key (GtkAction* action, DenemoScriptParam *param)
{
  adjustmeasurewidth (Denemo.project->si, -10);
}

/**
 * Enlarge the measure width by 10 pixels
 */
void
adjust_measure_more_width_key (GtkAction* action, DenemoScriptParam *param)
{
  adjustmeasurewidth (Denemo.project->si, 10);
}

/**
 * Reduce the staff height by 10 pixels
 */
void
adjust_staff_less_height_key (GtkAction* action, DenemoScriptParam *param)
{
  adjuststaffheight (Denemo.project->si, -10);
}

/**
 * Enlarge the staff height by 10 pixels
 */
void
adjust_staff_more_height_key (GtkAction* action, DenemoScriptParam *param)
{
  adjuststaffheight (Denemo.project->si, 10);

}

/**
 * Remove selection marker
 */
void
unset_selection_key (DenemoScriptParam *param)
{
  Denemo.project->si->markstaffnum = 0;
}

/**
 * Insert quarter note into score
 */
void
insert_chord_key (DenemoScriptParam *param)
{
  dnm_insertchord (Denemo.project, 2, Denemo.project->mode, FALSE);
}

void
go_to_key(gchar note, DenemoScriptParam *param)
{
  Denemo.project->last_source = INPUTKEYBOARD;
  shiftcursor (Denemo.project, ((note + 5 - 'A') % 7));
}

/**
 * Goto the nearest a
 *
 */
void
go_to_A_key (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->last_source = INPUTKEYBOARD;
  shiftcursor (Denemo.project, 5);
}

/**
 * Goto the nearest b
 *
 */
void
go_to_B_key (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->last_source = INPUTKEYBOARD;
  shiftcursor (Denemo.project, 6);
}

/**
 * Goto the nearest c
 *
 */
void
go_to_C_key (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->last_source = INPUTKEYBOARD;
  shiftcursor (Denemo.project, 0);
}

/**
 * Goto the nearest d
 *
 */
void
go_to_D_key (GtkAction* action, DenemoScriptParam *param)
{
  shiftcursor (Denemo.project, 1);
}

/**
 * Goto the nearest e
 *
 */
void
go_to_E_key (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->last_source = INPUTKEYBOARD;
  shiftcursor (Denemo.project, 2);
}

/**
 * Goto the nearest f
 *
 */
void
go_to_F_key (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->last_source = INPUTKEYBOARD;
  shiftcursor (Denemo.project, 3);
}

/**
 * Goto the nearest g
 *
 */
void
go_to_G_key (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->last_source = INPUTKEYBOARD;
  shiftcursor (Denemo.project, 4);
}


/**
 * Move cursor by amount or in EDIT mode change the note by the amount
 */
static void
octave_shift_key (DenemoScriptParam *param, gint amount)
{
  if (((DenemoStaff *) Denemo.project->si->currentstaff->data)->tone_store)
    {
      return;                   //FIXME create a function modify_tone, like delete_tone in pitchentry.c to do this sort of thing
    }
  else
    {
      if (Denemo.project->mode & (INPUTEDIT))
        {
          if (Denemo.project->si->currentobject)
            {
              objnode *thenote = nearestnote (Denemo.project->si->currentobject->data, Denemo.project->si->cursor_y);
              if (thenote)
                {
                  note copy = *((note *) thenote->data);
                  GList *direcs = ((note *) thenote->data)->directives;
                  store_for_undo_change (Denemo.project->si, Denemo.project->si->currentobject->data);
                  Denemo.project->si->undo_guard++;
                  delete_chordnote (Denemo.project);       //does not delete the directives.
                  Denemo.project->si->cursor_y = copy.mid_c_offset + amount;
                  insert_chordnote (Denemo.project);
                  changeenshift (Denemo.project->si->currentobject->data, Denemo.project->si->cursor_y, copy.enshift);
                  thenote = nearestnote (Denemo.project->si->currentobject->data, Denemo.project->si->cursor_y);
                  if (thenote)
                    ((note *) thenote->data)->directives = direcs;
                  Denemo.project->si->undo_guard--;
                  score_status (Denemo.project, TRUE);
                }
            }
        }
      else
        Denemo.project->si->cursor_y += amount;
    }
  if(!Denemo.non_interactive)
    gtk_widget_queue_draw(Denemo.scorearea);
}

/**
 * Move cursor an octave up
 */
void
octave_up_key (GtkAction* action, DenemoScriptParam *param)
{
  octave_shift_key (param, 7);
}




/**
 * Move cursor an octave down
 */
void
octave_down_key (GtkAction* action, DenemoScriptParam *param)
{
  octave_shift_key (param, -7);
}

/**
 * change to read only mode
 * only score traversal available FIXME wrong name
 */
void
default_mode (DenemoScriptParam *param)
{
  Denemo.project->mode ^= TRAVERSE;
  if (Denemo.project->mode & TRAVERSE)
    gtk_label_set_text (GTK_LABEL (Denemo.statuslabel), "Read Only");
  g_debug ("Mode %d\n", Denemo.project->mode);
  displayhelper (Denemo.project);
}




/**
 * Toggle into rest mode
 * 
 */
void
rest_toggle_key (GtkAction* action, DenemoScriptParam *param)
{
  RETURN_IF_NON_INTERACTIVE ();
  GtkAction *mode = gtk_ui_manager_get_action (Denemo.ui_manager, "/MainMenu/ModeMenu/Rest");
  gtk_action_activate (mode);
  displayhelper (Denemo.project);
  score_status(Denemo.project, TRUE);
}

/**
 * Toggle blank mode FIXME bitfields!!!
 *
 */
void
toggle_blank (GtkAction* action, DenemoScriptParam *param)
{
  RETURN_IF_NON_INTERACTIVE ();
  GtkAction *mode = gtk_ui_manager_get_action (Denemo.ui_manager, "/MainMenu/ModeMenu/Blank");
  gtk_action_activate (mode);
  displayhelper (Denemo.project);
  score_status(Denemo.project, TRUE);
}

/**
 * Add measure to end of staff / score
 */
void
append_measure_key (GtkAction* action, DenemoScriptParam *param)
{
  appendmeasures (Denemo.project->si, 1);
}

void
append_measure_score (GtkAction* action, DenemoScriptParam *param)
{
  appendmeasurestoentirescore (Denemo.project->si, 1);
}

/** 
 * Insert measure at the current 
 */
void
insert_measure_key (GtkAction* action, DenemoScriptParam *param)
{
  dnm_insertmeasures (Denemo.project->si, 1);
}

void
insert_chord_xkey (gint duration, DenemoScriptParam *param)
{
  dnm_insertchord (Denemo.project, duration, Denemo.project->mode, FALSE);
  displayhelper (Denemo.project);
  score_status(Denemo.project, TRUE);
}

void
insert_chord_0key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(0, param);
}

void
insert_chord_1key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(1, param);
}

void
insert_chord_2key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(2, param);
}

void
insert_chord_3key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(3, param);
}

void
insert_chord_4key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(4, param);
}

void
insert_chord_5key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(5, param);
}

void
insert_chord_6key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(6, param);
}

void
insert_chord_7key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(7, param);
}

void
insert_chord_8key (GtkAction* action, DenemoScriptParam *param)
{
  insert_chord_xkey(8, param);
}

void
insert_rest_xkey(gint duration, DenemoScriptParam* param)
{
  dnm_insertchord (Denemo.project, duration, INPUTREST, TRUE);
  displayhelper (Denemo.project);
  score_status(Denemo.project, TRUE);
}


void
insert_rest_0key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(0, param);
}

void
insert_rest_1key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(1, param);
}

void
insert_rest_2key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(2, param);
}

void
insert_rest_3key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(3, param);
}

void
insert_rest_4key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(4, param);
}

void
insert_rest_5key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(5, param);
}

void
insert_rest_6key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(6, param);
}

void
insert_rest_7key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(7, param);
}

void
insert_rest_8key (GtkAction* action, DenemoScriptParam *param)
{
  insert_rest_xkey(8, param);
}

void
insert_blankchord_xkey (gint duration, DenemoScriptParam *param)
{
  dnm_insertchord (Denemo.project, duration, INPUTBLANK, FALSE);
  displayhelper (Denemo.project);
  score_status(Denemo.project, TRUE);
}

void
insert_blankchord_0key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(0, param);
}

void
insert_blankchord_1key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(1, param);
}

void
insert_blankchord_2key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(2, param);
}

void
insert_blankchord_3key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(3, param);
}

void
insert_blankchord_4key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(4, param);
}

void
insert_blankchord_5key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(5, param);
}

void
insert_blankchord_6key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(6, param);
}

void
insert_blankchord_7key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(7, param);
}

void
insert_blankchord_8key (GtkAction* action, DenemoScriptParam *param)
{
  insert_blankchord_xkey(8, param);
}


void
insert_duplet (GtkAction* action, DenemoScriptParam *param)
{
  dnm_inserttuplet (Denemo.project, DUPLET);
}

void
insert_triplet (GtkAction* action, DenemoScriptParam *param)
{
  dnm_inserttuplet (Denemo.project, TRIPLET);
}

void
start_triplet (GtkAction* action, DenemoScriptParam *param)
{
  insertion_point (Denemo.project->si);
  object_insert (Denemo.project, newtupopen (2, 3));
}

void
end_tuplet (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, newtupclose ());
}

void
insert_quadtuplet (GtkAction* action, DenemoScriptParam *param)
{
  dnm_inserttuplet (Denemo.project, QUADTUPLET);
}

void
insert_quintuplet (GtkAction* action, DenemoScriptParam *param)
{
  dnm_inserttuplet (Denemo.project, QUINTUPLET);
}

void
insert_sextuplet (GtkAction* action, DenemoScriptParam *param)
{
  dnm_inserttuplet (Denemo.project, SEXTUPLET);
}

void
insert_septuplet (GtkAction* action, DenemoScriptParam *param)
{
  dnm_inserttuplet (Denemo.project, SEPTUPLET);
}



gboolean
add_tone_key (GtkAction* action, DenemoScriptParam *param)
{
  return insert_chordnote (Denemo.project);
}

gboolean
remove_tone_key (GtkAction* action, DenemoScriptParam *param)
{
  return delete_chordnote (Denemo.project);
}

void
deletepreviousobject (GtkAction* action, DenemoScriptParam *param)
{

  /* remove the object preceding the cursor, within the current measure */
  if (Denemo.project->si->cursor_x)
    {
      /* Then move the cursor back */
      movecursorleft (NULL, NULL);
      /* And delete */
      deleteobject (NULL, NULL);
      /* if you are following a rhythmic pattern then backup the pattern */
#define g  (Denemo.project->rstep)
      if ((Denemo.project->mode & (INPUTEDIT) && g))
        {
#define CURRP ((RhythmPattern *)Denemo.project->currhythm->data)
          g = g->prev;          /* list is circular - should we stop at beginning? */
          if (Denemo.project->cstep)
            {
              Denemo.project->cstep = Denemo.project->cstep->prev ? Denemo.project->cstep->prev : g_list_last (CURRP->clipboard)->data;
            }
          if (((RhythmElement *) g->data)->icon)
            {
              GtkWidget *label = LABEL (CURRP->button);
              gtk_label_set_markup (GTK_LABEL (label), ((RhythmElement *) g->data)->icon);
            }
#undef CURRP
#undef g
        }

    }
  else
    {                           /* go to the previous measure, go to end of it, and start deleting there */
      if (Denemo.project->si->currentmeasure->prev)
        {
          DenemoScriptParam param;

          do
            {
              movetomeasureleft (NULL, &param);
              //go to end
              while (Denemo.project->si->currentobject && (Denemo.project->si->currentobject->next))
                {
                  Denemo.project->si->currentobject = Denemo.project->si->currentobject->next;
                  Denemo.project->si->cursor_x++;
                }
            }
          while (param.status && !Denemo.project->si->currentobject);


          if (Denemo.project->si->currentobject)
            {
              movecursorright (NULL, NULL);
              deletepreviousobject (NULL, NULL);
            }
        }
    }
}

void
sharpen_key (GtkAction* action, DenemoScriptParam *param)
{
  DenemoObject *curmudelaobj = (DenemoObject *) (Denemo.project->si->currentobject ? Denemo.project->si->currentobject->data : NULL);

  if (curmudelaobj && curmudelaobj->type == STEMDIRECTIVE)
    change_stem_directive (Denemo.project->si, DENEMO_STEMUP);
  else
    incrementenshift (Denemo.project, 1);
}

void
stem_up (GtkAction* action, DenemoScriptParam *param)
{
  sharpen_key (action, param);
}

void
flatten_key (GtkAction* action, DenemoScriptParam *param)
{
  DenemoObject *curmudelaobj = (DenemoObject *) (Denemo.project->si->currentobject ? Denemo.project->si->currentobject->data : NULL);

  if (curmudelaobj && curmudelaobj->type == STEMDIRECTIVE)
    change_stem_directive (Denemo.project->si, DENEMO_STEMDOWN);
  else
    incrementenshift (Denemo.project, -1);
}

void
pending_sharpen (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->si->pending_enshift++;
  if (Denemo.project->si->pending_enshift > 2)
    Denemo.project->si->pending_enshift = 2;
  displayhelper (Denemo.project);
  score_status(Denemo.project, TRUE);
}

void
pending_flatten (GtkAction* action, DenemoScriptParam *param)
{
  Denemo.project->si->pending_enshift--;
  if (Denemo.project->si->pending_enshift < -2)
    Denemo.project->si->pending_enshift = -2;
  displayhelper (Denemo.project);
  score_status(Denemo.project, TRUE);
}

void
stem_down (GtkAction* action, DenemoScriptParam *param)
{
  flatten_key (action, param);
}

/* insert a duplicate note and tie to it */
void
tie_notes_key (GtkAction* action, DenemoScriptParam *param)
{
  DenemoObject *curmudelaobj = (DenemoObject *) (Denemo.project->si->currentobject ? Denemo.project->si->currentobject->data : NULL);

  /* Equals - toggle whether this note is tied */
  if (curmudelaobj && curmudelaobj->type == CHORD && ((chord *) curmudelaobj->object)->notes)
    {

      insertion_point (Denemo.project->si);
      object_insert (Denemo.project, dnm_clone_object (curmudelaobj));
      movecursorleft (NULL, NULL);
      movecursorleft (NULL, NULL);
      toggle_tie (NULL, NULL);
      movecursorright (NULL, NULL);
      movecursorright (NULL, NULL);
    }
}

void
add_dot_key (GtkAction* action, DenemoScriptParam *param)
{
  changedots (Denemo.project->si, 1);
}

void
remove_dot_key (GtkAction* action, DenemoScriptParam *param)
{
  changedots (Denemo.project->si, -1);
}


void
force_cautionary (GtkAction* action, DenemoScriptParam *param)
{
  DenemoObject *theobj = Denemo.project->si->currentobject ? (DenemoObject *) Denemo.project->si->currentobject->data : NULL;
  if (theobj && theobj->type == CHORD)
    caution (Denemo.project->si);
}

void
change_pitch (GtkAction* action, DenemoScriptParam *param)
{
  if (Denemo.project->mode & INPUTEDIT)
    {
      //DenemoObject *theobj =
      //  si->currentobject ? (DenemoObject *) si->currentobject->data : NULL;
      delete_chordnote (Denemo.project);
      insert_chordnote (Denemo.project);
    }
  // addtone(theobj, si->cursor_y, si->cursoraccs[si->staffletter_y],
  //       si->cursorclef);

}




/**
 * Wrapper function to create new treble clef and insert into the score
 */
void
newcleftreble (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_TREBLE_CLEF));
}

/**
 * Wrapper function to create new bass clef and insert into the score
 */
void
newclefbass (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_BASS_CLEF));
}

/**
 * Wrapper function to create new alto clef and insert into the score
 */
void
newclefalto (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_ALTO_CLEF));
}

/**
 * Wrapper function to create new treble_8 clef and insert into the score
 */
void
newclefg8 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_G_8_CLEF));
}

/**
 * Wrapper function to create new bass_8 clef and insert into the score
 */
void
newcleff8 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_F_8_CLEF));
}

/**
 * Wrapper function to create new tenor clef and insert into the score
 */
void
newcleftenor (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_TENOR_CLEF));
}

/**
 * Wrapper function to create new soprano clef and insert into the score
 */
void
newclefsoprano (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_SOPRANO_CLEF));
}

/**
 * Wrapper function to create new french clef and insert into the score
 */
void
newcleffrench (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newclefobj (DENEMO_FRENCH_CLEF));
}

/**
 * Wrapper function to create new 4/4 time sig and insert into the score
 */
void
newtimesig44 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (4, 4));
}

/**
 * Wrapper function to create new 2/4 time sig and insert into the score
 */
void
newtimesig24 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (2, 4));
}

/**
 * Wrapper function to create new 3/4 time sig and insert into the score
 */
void
newtimesig34 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (3, 4));
}

/**
 * Wrapper function to create new 6/4 time sig and insert into the score
 */
void
newtimesig64 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (6, 4));
}

/**
 * Wrapper function to create new 5/4 time sig and insert into the score
 */
void
newtimesig54 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (5, 4));
}

/**
 * Wrapper function to create new 3/8 time sig and insert into the score
 */
void
newtimesig38 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (3, 8));
}

/**
 * Wrapper function to create new 6/8 time sig and insert into the score
 */
void
newtimesig68 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (6, 8));
}

/**
 * Wrapper function to create new 9/8 time sig and insert into the score
 */
void
newtimesig98 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (9, 8));
}

/**
 * Wrapper function to create new 12/8 time sig and insert into the score
 */
void
newtimesig128 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (12, 8));
}

/**
 * Wrapper function to create new 2/2 time sig and insert into the score
 */
void
newtimesig22 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (2, 2));
}

/**
 * Wrapper function to create new 3/2 time sig and insert into the score
 */
void
newtimesig32 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (3, 2));
}

/**
 * Wrapper function to create new 4/2 time sig and insert into the score
 */
void
newtimesig42 (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newtimesigobj (4, 2));
}



void
newkeysigcmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (0, 1, 0));
}

void
newkeysiggmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (1, 1, 0));
}

void
newkeysigdmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (2, 1, 0));
}

void
newkeysigamaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (3, 1, 0));
}

void
newkeysigemaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (4, 1, 0));
}

void
newkeysigbmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (5, 1, 0));
}

void
newkeysigfsharpmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (6, 1, 0));
}

void
newkeysigcsharpmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (7, 1, 0));
}

void
newkeysigfmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-1, 1, 0));
}

void
newkeysigbflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-2, 1, 0));
}

void
newkeysigeflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-3, 1, 0));
}

void
newkeysigaflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-4, 1, 0));
}

void
newkeysigdflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-5, 1, 0));
}

void
newkeysiggflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-6, 1, 0));
}

void
newkeysigcflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-7, 1, 0));
}

void
newkeysigamin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (0, 0, 0));
}

void
newkeysigemin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (1, 0, 0));
}

void
newkeysigbmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (2, 0, 0));
}

void
newkeysigfsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (3, 0, 0));
}

void
newkeysigcsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (4, 0, 0));
}

void
newkeysiggsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (5, 0, 0));
}

void
newkeysigdsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (6, 0, 0));
}

void
newkeysigasharpmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (7, 0, 0));
}

void
newkeysigdmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-1, 0, 0));
}

void
newkeysiggmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-2, 0, 0));
}

void
newkeysigcmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-3, 0, 0));
}

void
newkeysigfmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-4, 0, 0));
}

void
newkeysigbflatmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-5, 0, 0));
}

void
newkeysigeflatmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-6, 0, 0));
}

void
newkeysigaflatmin (GtkAction* action, DenemoScriptParam *param)
{
  object_insert (Denemo.project, dnm_newkeyobj (-7, 0, 0));
}


//Functions to set the initial key signature
void
setkeysigcmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 0, 1);
  //displayhelper(si);
}

void
setkeysiggmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 1, 1);
  //displayhelper(si);
}

void
setkeysigdmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 2, 1);
}

void
setkeysigamaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 3, 1);
}

void
setkeysigemaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 4, 1);
}

void
setkeysigbmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 5, 1);
}

void
setkeysigfsharpmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 6, 1);
}

void
setkeysigcsharpmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 7, 1);
}

void
setkeysigfmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -1, 1);
}

void
setkeysigbflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -2, 1);
}

void
setkeysigeflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -3, 1);
}

void
setkeysigaflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -4, 1);
}

void
setkeysigdflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -5, 1);
}

void
setkeysiggflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -6, 1);
}

void
setkeysigcflatmaj (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -7, 1);
}

void
setkeysigamin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 0, 0);
}

void
setkeysigemin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 1, 0);
}

void
setkeysigbmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 2, 0);
}

void
setkeysigfsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 3, 0);
}

void
setkeysigcsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 4, 0);
}

void
setkeysiggsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 5, 0);
}

void
setkeysigdsharpmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 6, 0);
}

void
setkeysigasharpmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, 7, 0);
}

void
setkeysigdmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -1, 0);
}

void
setkeysiggmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -2, 0);
}

void
setkeysigcmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -3, 0);
}

void
setkeysigfmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -4, 0);
}

void
setkeysigbflatmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -5, 0);
}

void
setkeysigeflatmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -6, 0);
}

void
setkeysigaflatmin (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  dnm_setinitialkeysig (curstaff, -7, 0);
}


void
settimesig22 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 2, 2, TRUE);
}

void
settimesig42 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 4, 2, TRUE);
}

void
settimesig32 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 3, 2, TRUE);
}

void
settimesig44 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 4, 4, TRUE);
}

void
settimesig54 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 5, 4, TRUE);
}

void
settimesig24 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 2, 4, TRUE);
}

void
settimesig34 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 3, 4, TRUE);
}

void
settimesig68 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 6, 8, TRUE);
}

void
settimesig128 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 12, 8, TRUE);
}

void
settimesig38 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 3, 8, TRUE);
}

void
settimesig98 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 9, 8, TRUE);
}

void
settimesig64 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialtimesig (Denemo.project->si, curstaff, 6, 4, TRUE);
}

void
setcleftreble (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_TREBLE_CLEF);
}

void
setclefbass (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_BASS_CLEF);
}

void
setclefg8 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_G_8_CLEF);
}

void
setcleff8 (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_F_8_CLEF);
}

void
setclefalto (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_ALTO_CLEF);
}

void
setcleftenor (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_TENOR_CLEF);
}

void
setclefsoprano (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_SOPRANO_CLEF);
}

void
setcleffrench (GtkAction* action, DenemoScriptParam *param)
{
  DenemoStaff *curstaff = (DenemoStaff *) Denemo.project->si->currentstaff->data;
  if (curstaff)
    dnm_setinitialclef (Denemo.project->si, curstaff, DENEMO_FRENCH_CLEF);
}

/*******************************************************************************
* DURATION COMMANDS
******************************************************************************/

void InsertRest(gint duration){
  highlight_rest(Denemo.project, duration);
  gint mode = Denemo.project->mode;
  Denemo.project->mode = INPUTINSERT|INPUTREST;
  insert_chord_xkey(duration, NULL);
  Denemo.project->mode = mode;
  score_status(Denemo.project, TRUE);
  displayhelper(Denemo.project);
}

void InsertDur(gint duration){
  highlight_duration(Denemo.project, duration);
  gint mode = Denemo.project->mode;
  Denemo.project->mode = INPUTINSERT|INPUTNORMAL;
  insert_chord_xkey(duration, NULL);
  Denemo.project->mode = mode;
  score_status(Denemo.project, TRUE);
  displayhelper(Denemo.project);
}

void ChangeDur(gint duration){
  gint mode = Denemo.project->mode;
  gboolean appending = Denemo.project->si->cursor_appending;
  if(appending)
    movecursorleft(NULL, NULL);
  Denemo.project->mode = INPUTEDIT|INPUTNORMAL;
  insert_chord_xkey(duration, NULL);
  Denemo.project->mode = mode;
  if(appending)
    movecursorright(NULL, NULL);
  score_status(Denemo.project, TRUE);
  displayhelper(Denemo.project);
}

void SetDur(gint duration){
  highlight_duration(Denemo.project, duration);
}

void Dur (gint duration) {
  DenemoProject *gui = Denemo.project;
 if(Denemo.project->mode&INPUTINSERT)
   highlight_duration(Denemo.project, duration);
 else
 if( !(Denemo.project->mode&INPUTRHYTHM) && (Denemo.project->mode&INPUTEDIT) && (!Denemo.project->si->cursor_appending))
   ChangeDur (duration);
else {
 insert_chord_xkey(duration, NULL);
   highlight_duration(Denemo.project, duration);
  score_status(Denemo.project, TRUE);
 displayhelper(Denemo.project);
 }
}

/*******************************************************************************
* NOTE COMMANDS
******************************************************************************/

void ChangeTo(gchar note){
  gboolean appending = Denemo.project->si->cursor_appending;
  if(appending)
    movecursorleft(NULL, NULL);
  gint mode = Denemo.project->mode;
  Denemo.project->mode = INPUTEDIT|INPUTNORMAL;
  go_to_key(note, NULL);
  Denemo.project->mode = mode;
  if(appending)
    movecursorright(NULL, NULL);
  score_status(Denemo.project, TRUE);
  displayhelper(Denemo.project);
}

void MoveTo(gchar note){
  gint mode = Denemo.project->mode;
  Denemo.project->mode = INPUTCLASSIC|INPUTNORMAL;
  go_to_key(note, NULL);
  Denemo.project->mode = mode;
  displayhelper(Denemo.project);
}

void Insert(gchar note){
  gint mode = Denemo.project->mode;
  Denemo.project->mode = INPUTINSERT|INPUTNORMAL;
  go_to_key(note, NULL);
  Denemo.project->mode = mode;
  score_status(Denemo.project, TRUE);
  displayhelper(Denemo.project);
}

void AddNote(gchar note){
  movecursorright(NULL, NULL);
  gint mode = Denemo.project->mode;
  Denemo.project->mode = INPUTINSERT|INPUTNORMAL;
  go_to_key(note, NULL);
  Denemo.project->mode = mode;
  movecursorleft(NULL, NULL);
  score_status(Denemo.project, TRUE);
  displayhelper(Denemo.project);
}

void Add(gchar note){
  gint mode = Denemo.project->mode;
  Denemo.project->mode = INPUTCLASSIC|INPUTNORMAL;
  go_to_key(note, NULL);
  add_tone_key(NULL, NULL);
  Denemo.project->mode = mode;
  score_status(Denemo.project, TRUE);
  displayhelper(Denemo.project);
}

void Dur0(GtkAction *action, gpointer param) {
  Dur(0);
}
void ChangeDur0(GtkAction *action, gpointer param){
  ChangeDur(0);
}
void InsertDur0(GtkAction *action, gpointer param){
  InsertDur(0);
}
void InsertRest0(GtkAction *action, gpointer param){
  InsertRest(0);
}
void SetDur0(GtkAction *action, gpointer param){
  SetDur(0);
}
void Dur1(GtkAction *action, gpointer param) {
  Dur(1);
}
void ChangeDur1(GtkAction *action, gpointer param){
  ChangeDur(1);
}
void InsertDur1(GtkAction *action, gpointer param){
  InsertDur(1);
}
void InsertRest1(GtkAction *action, gpointer param){
  InsertRest(1);
}
void SetDur1(GtkAction *action, gpointer param){
  SetDur(1);
}
void Dur2(GtkAction *action, gpointer param) {
  Dur(2);
}
void ChangeDur2(GtkAction *action, gpointer param){
  ChangeDur(2);
}
void InsertDur2(GtkAction *action, gpointer param){
  InsertDur(2);
}
void InsertRest2(GtkAction *action, gpointer param){
  InsertRest(2);
}
void SetDur2(GtkAction *action, gpointer param){
  SetDur(2);
}
void Dur3(GtkAction *action, gpointer param) {
  Dur(3);
}
void ChangeDur3(GtkAction *action, gpointer param){
  ChangeDur(3);
}
void InsertDur3(GtkAction *action, gpointer param){
  InsertDur(3);
}
void InsertRest3(GtkAction *action, gpointer param){
  InsertRest(3);
}
void SetDur3(GtkAction *action, gpointer param){
  SetDur(3);
}
void Dur4(GtkAction *action, gpointer param) {
  Dur(4);
}
void ChangeDur4(GtkAction *action, gpointer param){
  ChangeDur(4);
}
void InsertDur4(GtkAction *action, gpointer param){
  InsertDur(4);
}
void InsertRest4(GtkAction *action, gpointer param){
  InsertRest(4);
}
void SetDur4(GtkAction *action, gpointer param){
  SetDur(4);
}
void Dur5(GtkAction *action, gpointer param) {
  Dur(5);
}
void ChangeDur5(GtkAction *action, gpointer param){
  ChangeDur(5);
}
void InsertDur5(GtkAction *action, gpointer param){
  InsertDur(5);
}
void InsertRest5(GtkAction *action, gpointer param){
  InsertRest(5);
}
void SetDur5(GtkAction *action, gpointer param){
  SetDur(5);
}
void Dur6(GtkAction *action, gpointer param) {
  Dur(6);
}
void ChangeDur6(GtkAction *action, gpointer param){
  ChangeDur(6);
}
void InsertDur6(GtkAction *action, gpointer param){
  InsertDur(6);
}
void InsertRest6(GtkAction *action, gpointer param){
  InsertRest(6);
}
void SetDur6(GtkAction *action, gpointer param){
  SetDur(6);
}
void Dur7(GtkAction *action, gpointer param) {
  Dur(7);
}
void ChangeDur7(GtkAction *action, gpointer param){
  ChangeDur(7);
}
void InsertDur7(GtkAction *action, gpointer param){
  InsertDur(7);
}
void InsertRest7(GtkAction *action, gpointer param){
  InsertRest(7);
}
void SetDur7(GtkAction *action, gpointer param){
  SetDur(7);
}
void Dur8(GtkAction *action, gpointer param) {
  Dur(8);
}
void ChangeDur8(GtkAction *action, gpointer param){
  ChangeDur(8);
}
void InsertDur8(GtkAction *action, gpointer param){
  InsertDur(8);
}
void InsertRest8(GtkAction *action, gpointer param){
  InsertRest(8);
}
void SetDur8(GtkAction *action, gpointer param){
  SetDur(8);
}
void InsertA(GtkAction *action, gpointer param){
  Insert('A');
}
void AddNoteA(GtkAction *action, gpointer param){
  AddNote('A');
}
void AddA(GtkAction *action, gpointer param){
  Add('A');
}
void ChangeToA(GtkAction *action, gpointer param){
  ChangeTo('A');
}
void MoveToA(GtkAction *action, gpointer param){
  MoveTo('A');
}
void InsertB(GtkAction *action, gpointer param){
  Insert('B');
}
void AddNoteB(GtkAction *action, gpointer param){
  AddNote('B');
}
void AddB(GtkAction *action, gpointer param){
  Add('B');
}
void ChangeToB(GtkAction *action, gpointer param){
  ChangeTo('B');
}
void MoveToB(GtkAction *action, gpointer param){
  MoveTo('B');
}
void InsertC(GtkAction *action, gpointer param){
  Insert('C');
}
void AddNoteC(GtkAction *action, gpointer param){
  AddNote('C');
}
void AddC(GtkAction *action, gpointer param){
  Add('C');
}
void ChangeToC(GtkAction *action, gpointer param){
  ChangeTo('C');
}
void MoveToC(GtkAction *action, gpointer param){
  MoveTo('C');
}
void InsertD(GtkAction *action, gpointer param){
  Insert('D');
}
void AddNoteD(GtkAction *action, gpointer param){
  AddNote('D');
}
void AddD(GtkAction *action, gpointer param){
  Add('D');
}
void ChangeToD(GtkAction *action, gpointer param){
  ChangeTo('D');
}
void MoveToD(GtkAction *action, gpointer param){
  MoveTo('D');
}
void InsertE(GtkAction *action, gpointer param){
  Insert('E');
}
void AddNoteE(GtkAction *action, gpointer param){
  AddNote('E');
}
void AddE(GtkAction *action, gpointer param){
  Add('E');
}
void ChangeToE(GtkAction *action, gpointer param){
  ChangeTo('E');
}
void MoveToE(GtkAction *action, gpointer param){
  MoveTo('E');
}
void InsertF(GtkAction *action, gpointer param){
  Insert('F');
}
void AddNoteF(GtkAction *action, gpointer param){
  AddNote('F');
}
void AddF(GtkAction *action, gpointer param){
  Add('F');
}
void ChangeToF(GtkAction *action, gpointer param){
  ChangeTo('F');
}
void MoveToF(GtkAction *action, gpointer param){
  MoveTo('F');
}
void InsertG(GtkAction *action, gpointer param){
  Insert('G');
}
void AddNoteG(GtkAction *action, gpointer param){
  AddNote('G');
}
void AddG(GtkAction *action, gpointer param){
  Add('G');
}
void ChangeToG(GtkAction *action, gpointer param){
  ChangeTo('G');
}
void MoveToG(GtkAction *action, gpointer param){
  MoveTo('G');
}