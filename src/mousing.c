/* mousing.c
 * callback functions for handling mouse clicks, drags, etc.
 *
 *  for Denemo, a gtk+ frontend to GNU Lilypond
 *  (c) 2000-2005 Matthew Hiller
 */

#include "commandfuncs.h"
#include "kbd-custom.h"
#include "staffops.h"
#include "utils.h"
#include "selectops.h"
#include "lilydirectives.h"
#include "lyric.h"
#include "moveviewport.h"
#include "mousing.h"


static gboolean lh_down;

/**
 * Get the mid_c_offset of an object or click from its height relative
 * to the top of the staff.  
 */
gint
offset_from_height (gdouble height, enum clefs clef)
{
  /* Offset from the top of the staff, in half-tones.  */
  gint half_tone_offset = ((gint) (height / HALF_LINE_SPACE+((height>0)?0.5:-0.5)));

#define R(x) return x - half_tone_offset

  switch (clef)
    {
    case DENEMO_TREBLE_CLEF:
      R (10);
      break;
    case DENEMO_BASS_CLEF:
      R (-2);
      break;
    case DENEMO_ALTO_CLEF:
      R (4);
      break;
    case DENEMO_G_8_CLEF:
      R (3);
      break;
    case DENEMO_TENOR_CLEF:
      R (2);
      break;
    case DENEMO_SOPRANO_CLEF:
      R (8);
      break;
    default:
      R (0);
      break;
    }
#undef R
  return 0;
}


static gdouble get_click_height(DenemoGUI * gui, gdouble y) {
  gdouble click_height;
  gint staffs_from_top;
  staffs_from_top = 0;
  GList *curstaff;
  DenemoStaff *staff;
  gint extra_space = 0;
  gint space_below = 0;
  curstaff = g_list_nth(gui->si->thescore,gui->si->top_staff-1);

  if(((DenemoStaff *)(gui->si->currentstaff->data))->voicenumber != 1)
    staffs_from_top--;

  for(  curstaff = g_list_nth(gui->si->thescore,gui->si->top_staff-1) ; curstaff;curstaff=curstaff->next) {
    //g_print("before extra space %d\n", extra_space);
    staff = (DenemoStaff *) curstaff->data;
    if(staff->voicenumber == 1)
      extra_space += (staff->space_above) + space_below;
    if(curstaff == gui->si->currentstaff)
      break;
    
    if(staff->voicenumber == 1){

      space_below = 0;
      staffs_from_top++;
    }
    space_below = MAX(space_below, ((staff->space_below) + (staff->verses?LYRICS_HEIGHT:0)));
    //g_print("after extra space %d space_below %d\n", extra_space, space_below);
  }

  click_height =
    y - (gui->si->staffspace * staffs_from_top + gui->si->staffspace / 4 + extra_space);
  //  g_print("top staff is %d total %d staffs from top is %d click %f\n", gui->si->top_staff, extra_space, staffs_from_top, click_height);

  return click_height;



}

/**
 * Set the cursor's y position from a mouse click
 *
 */
void
set_cursor_y_from_click (DenemoGUI * gui, gdouble y)
{
  /* Click height relative to the top of the staff.  */
  gdouble click_height = get_click_height(gui, y);
  gui->si->cursor_y =
    offset_from_height (click_height, (enum clefs) gui->si->cursorclef);
  gui->si->staffletter_y = offsettonumber (gui->si->cursor_y);
}

struct placement_info
{
  gint staff_number, measure_number, cursor_x;
  staffnode *the_staff;
  measurenode *the_measure;
  objnode *the_obj;
  gboolean nextmeasure;
};

/* find the primary staff of the current staff, return its staffnum */
static gint primary_staff(DenemoScore *si) {
  GList *curstaff;
  for(curstaff = si->currentstaff;  curstaff && ((DenemoStaff *) curstaff->data)->voicenumber!=1;curstaff=curstaff->prev)
   ;//do nothing
  //g_print("The position is %d\n", 1+g_list_position(si->thescore, curstaff));
  return 1+g_list_position(si->thescore, curstaff);
}


/* find which staff in si the height y lies in, return the staff number (not counting non-primary staffs ie voices) */

static gint staff_at (gint y, DenemoScore *si) {
  GList *curstaff;
  gint space = 0;
  gint count;
  gint ret;
  for(curstaff = g_list_nth(si->thescore, si->top_staff-1), count=0; curstaff && y>space;curstaff=curstaff->next) {
    DenemoStaff *staff = (DenemoStaff *) curstaff->data;

    count++;
    if(staff->voicenumber == 1)
      space += (staff)->space_above +
	(staff)->space_below + si->staffspace; 
    //g_print("y %d and space %d count = %d\n",y,space, count);
  } 

  if(y<=1)
    ret = 1;
  ret = count+si->top_staff-1;
  if(ret==primary_staff(si))
    ret = si->currentstaffnum;
  return ret>0?ret:1;
}

/**
 * Gets the position from the clicked position
 *
 */
static void
get_placement_from_coordinates (struct placement_info *pi,
				gdouble x, gdouble y, gint leftmeasurenum, gint rightmeasurenum )
{
  DenemoScore *si = Denemo.gui->si;
  GList *mwidthiterator = g_list_nth (si->measurewidths,
				      leftmeasurenum - 1);
  objnode *obj_iterator;
  gint x_to_explain = (gint) (x);

  pi->staff_number = staff_at((gint)y, si);
  //g_print("L/R %d %d got staff number %d\n", leftmeasurenum, rightmeasurenum, pi->staff_number); 
  pi->measure_number = leftmeasurenum;
  x_to_explain -= (KEY_MARGIN + si->maxkeywidth + SPACE_FOR_TIME);
  //g_print("Explaining %d\n", x_to_explain);
  while (x_to_explain > GPOINTER_TO_INT (mwidthiterator->data)
	 && pi->measure_number < rightmeasurenum)
    {
      x_to_explain -= (GPOINTER_TO_INT (mwidthiterator->data)
		       + SPACE_FOR_BARLINE);
      mwidthiterator = mwidthiterator->next;
      pi->measure_number++;
    }
  //g_print("got to measure %d\n", pi->measure_number);
  pi->nextmeasure = (x_to_explain > GPOINTER_TO_INT (mwidthiterator->data)
		     && pi->measure_number >= rightmeasurenum);
    
  pi->the_staff = g_list_nth (si->thescore, pi->staff_number - 1);
  pi->the_measure
    = nth_measure_node_in_staff (pi->the_staff, pi->measure_number - 1);
  if (pi->the_measure != NULL){ /*check to make sure user did not click on empty space*/
	  obj_iterator = (objnode *) pi->the_measure->data;
	  pi->cursor_x = 0;
	  pi->the_obj = NULL;
	  if (obj_iterator)
	    {
	      DenemoObject *current, *next;

	      for (; obj_iterator->next;
		   obj_iterator = obj_iterator->next, pi->cursor_x++)
		{
		  current = (DenemoObject *) obj_iterator->data;
		  next = (DenemoObject *) obj_iterator->next->data;
		  /* This comparison neatly takes care of two possibilities:

		     1) That the click was to the left of current, or

		     2) That the click was between current and next, but
		     closer to current.

		     Do the math - it really does work out.  */

		  //???modify current->x by gx where graphic_override is set????
		  if (x_to_explain - (current->x + current->minpixelsalloted)
		      < next->x - x_to_explain)
		    {
		      pi->the_obj = obj_iterator;
		      break;
		    }
		}
	      if (!obj_iterator->next)
		/* That is, we exited the loop normally, not through a break.  */
		{
		  DenemoObject *current = (DenemoObject *) obj_iterator->data;
		  pi->the_obj = obj_iterator;
		  /* The below makes clicking to get the object at the end of
		     a measure (instead of appending after it) require
		     precision.  This may be bad; tweak me if necessary.  */
		  if (x_to_explain > current->x + current->minpixelsalloted)
		    pi->cursor_x++;
		}
	    }
	  //g_print("got to cursor x %d\n", pi->cursor_x);
  }
}


void assign_cursor(guint state, guint cursor_num) {
  guint *cursor_state = g_new(guint,1);
  *cursor_state = state;
  //g_print("Storing cursor %x for state %x in hash table %p\n", cursor_num, state, Denemo.map->cursors );  
  GdkCursor *cursor = gdk_cursor_new(cursor_num);
  g_assert(cursor);
  g_hash_table_insert(Denemo.map->cursors, cursor_state, cursor);
}

void
set_cursor_for(guint state) {
  gint the_state = state;
  GdkCursor *cursor = g_hash_table_lookup(Denemo.map->cursors, &the_state);
  //g_print("looked up %x in %p got cursor %p which is number %d\n", state, Denemo.map->cursors,  cursor, cursor?cursor->type:-1);
  if(cursor)
    gdk_window_set_cursor(Denemo.window->window, cursor);
   else 
     gdk_window_set_cursor(Denemo.window->window, gdk_cursor_new(GDK_LEFT_PTR));//FIXME? does this take time/hog memory
}


/* appends the name(s) for modifier mod to ret->str */

void append_modifier_name(GString *ret, gint mod) {
  gint i;
  static const gchar* names[]= {
 "Shift"   ,
  "CapsLock"	   ,
  "Control" ,
  "Alt"	   ,
  "NumLock"	 ,
  "MOD3"	   ,
  "Penguin"	   ,
  "AltGr"
  };
  for(i=0;i<DENEMO_NUMBER_MODIFIERS;i++)
    if((1<<i)&mod)
      g_string_append_printf(ret, "%s%s", "-",names[i]);
  g_string_append_printf(ret, "%s", mod?"":"");
}

/* returns a newly allocated GString containing a shortcut name */
GString* mouse_shortcut_name(gint mod,  mouse_gesture gesture, gboolean left) {

  GString *ret = g_string_new((gesture==GESTURE_PRESS)?(left?"PrsL":"PrsR"):((gesture==GESTURE_RELEASE)?(left?"RlsL":"RlsR"):(left?"MveL":"MveR")));

  append_modifier_name(ret, mod);
  //g_print("Returning %s for mod %d\n", ret->str, mod);
  return ret;

}


/* perform an action for mouse-click stored with shortcuts */
static void  
perform_command(gint modnum, mouse_gesture press, gboolean left)
{
  GString *modname = mouse_shortcut_name(modnum, press, left);
  gint command_idx = lookup_command_for_keybinding_name (Denemo.map, modname->str);
  if(press != GESTURE_MOVE){
    if(!Denemo.prefs.strictshortcuts){
      if(command_idx<0) {
	g_string_free(modname, TRUE);
	modname = mouse_shortcut_name(modnum&(~GDK_LOCK_MASK/*CapsLock*/), press, left);
	command_idx = lookup_command_for_keybinding_name (Denemo.map, modname->str);  
      }
      if(command_idx<0){
	g_string_free(modname, TRUE);
	modname = mouse_shortcut_name(modnum&(~GDK_MOD2_MASK/*NumLock*/), press, left);
	command_idx = lookup_command_for_keybinding_name (Denemo.map, modname->str);
      }
      if(command_idx<0){
	g_string_free(modname, TRUE);
	modname = mouse_shortcut_name(modnum&(~(GDK_LOCK_MASK|GDK_MOD2_MASK)), press, left);
	command_idx = lookup_command_for_keybinding_name (Denemo.map, modname->str);
      }
    }
  }


  if(command_idx>=0) {
    execute_callback_from_idx (Denemo.map, command_idx);
    displayhelper (Denemo.gui);
  }
  g_string_free(modname, TRUE);
}  

static gboolean selecting = FALSE;
static gboolean dragging_separator = FALSE;


static gboolean change_staff(DenemoScore *si, gint num, GList *staff) {
  if(si->currentstaffnum==num)
    return FALSE;
  hide_lyrics();
  si->currentstaffnum = num;
  si->currentstaff = staff;
  show_lyrics();
  return TRUE;
}

static void
transform_coords(double* x, double* y) {
  DenemoGUI *gui = Denemo.gui;
  gint line_height = gui->scorearea->allocation.height*gui->si->system_height;
  gint line_num = ((int)*y)/line_height;
  *y -= line_num * line_height;
  *x /= gui->si->zoom;
  *y /= gui->si->zoom;
  // *x += ((double)line_num * gui->si->widthtoworkwith / ((int)(1/gui->si->system_height))) - 1.0* (line_num?(double)LEFT_MARGIN:0.0);
}


gint 
scorearea_leave_event(GtkWidget *widget, GdkEventCrossing *event) {
   gdk_window_set_cursor(Denemo.window->window, gdk_cursor_new(GDK_LEFT_PTR));//FIXME? does this take time/hog memory
   return FALSE;//allow other handlers (specifically the pitch entry one)
}

/**
 * Mouse motion callback 
 *
 */
gint
scorearea_motion_notify (GtkWidget * widget, GdkEventButton * event)
{
  DenemoGUI *gui = Denemo.gui;
  if(gui==NULL)
    return FALSE;
  if(gui->scorearea==NULL)
    return FALSE;
  gint line_height = gui->scorearea->allocation.height*gui->si->system_height;
  if (event->y < 0)
    event->y = 0.0;
  gint line_num = ((int)event->y)/line_height;
  
#define DENEMO_MINIMUM_SYSTEM_HEIGHT (0.01)


  if(dragging_separator) {
    gui->si->system_height =  event->y/gui->scorearea->allocation.height;
    if(gui->si->system_height<DENEMO_MINIMUM_SYSTEM_HEIGHT)
      gui->si->system_height = DENEMO_MINIMUM_SYSTEM_HEIGHT;
    if(gui->si->system_height>1.0)
      gui->si->system_height = 1.0;
    scorearea_configure_event(gui->scorearea, NULL);
    gtk_widget_queue_draw (gui->scorearea);
    return TRUE;
  }

  if(line_height - ((int)event->y - 8)%line_height<12)
    gdk_window_set_cursor(Denemo.window->window, gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW));
  else
    gdk_window_set_cursor(Denemo.window->window, gdk_cursor_new(GDK_LEFT_PTR));//FIXME? does this take time/hog memory

  transform_coords(&event->x, &event->y);
  //  g_print("Marked %d\n", gui->si->markstaffnum);
  if(gui->lefts[line_num] == 0)
    return TRUE;


  if(event->x<LEFT_MARGIN) {
    struct placement_info pi; //FIXME duplicate code
    get_placement_from_coordinates (&pi, event->x, event->y, gui->lefts[line_num],gui->rights[line_num]);
    if(pi.staff_number==gui->si->currentstaffnum) {
      gint offset = (gint)get_click_height(gui, event->y);
      if(offset<STAFF_HEIGHT/2) {
	if(((DenemoStaff*)gui->si->currentstaff->data)->staff_directives)
	  gtk_menu_popup (((DenemoStaff*)gui->si->currentstaff->data)->staffmenu, NULL, NULL, NULL, NULL,0, gtk_get_current_event_time()) ;
      }
    }
  }

    if (lh_down || (selecting && gui->si->markstaffnum)){
      struct placement_info pi; 
      if (event->y < 0)
	get_placement_from_coordinates (&pi, event->x, 0, gui->lefts[line_num],gui->rights[line_num]);
      else
	get_placement_from_coordinates (&pi, event->x, event->y, gui->lefts[line_num],gui->rights[line_num]);
      if (pi.the_measure != NULL){ /*don't place cursor in a place that is not there*/
	change_staff(gui->si, pi.staff_number, pi.the_staff);
	gui->si->currentmeasurenum = pi.measure_number;
	gui->si->currentmeasure = pi.the_measure;
	gui->si->currentobject = pi.the_obj;
	gui->si->cursor_x = pi.cursor_x;
	gui->si->cursor_appending
	  =
	  (gui->si->cursor_x ==
	   (gint) (g_list_length ((objnode *) gui->si->currentmeasure->data)));
	
	set_cursor_y_from_click (gui, event->y);
	if(lh_down & !selecting){
	  set_mark(gui);
	  selecting=TRUE;
	}
	calcmarkboundaries (gui->si);
	if(event->state&(GDK_BUTTON1_MASK|GDK_BUTTON2_MASK|GDK_BUTTON3_MASK))
	   perform_command(event->state, GESTURE_MOVE, event->state&GDK_BUTTON1_MASK);

	/* redraw to show new cursor position  */
	gtk_widget_queue_draw (gui->scorearea);
      }
    }
  return TRUE;
}


/**
 * Mouse button press callback 
 *
 */
gint
scorearea_button_press (GtkWidget * widget, GdkEventButton * event)
{
  DenemoGUI *gui = Denemo.gui;
  //if the cursor is at a system separator start dragging it
  gint line_height = gui->scorearea->allocation.height*gui->si->system_height;
  gint line_num = ((int)event->y)/line_height;
  //g_print("diff %d\n", line_height - ((int)event->y)%line_height);
  if(dragging_separator == FALSE)
  if(line_height - ((int)event->y - 8)%line_height<12) {
    dragging_separator = TRUE;
    return TRUE;
  }
  dragging_separator = FALSE;
  //g_print("before %f %f\n", event->x, event->y);
  transform_coords(&event->x, &event->y);
  //g_print("after %f %f\n", event->x, event->y);

  struct placement_info pi;
  gboolean left = (event->button != 3);
  gtk_widget_grab_focus(widget);
  gint key = gui->si->maxkeywidth;
  gint cmajor = key?0:5;//allow some area for keysig in C-major
  if(left && (gui->si->leftmeasurenum>1) && (event->x<KEY_MARGIN+SPACE_FOR_TIME+key)  && (event->x>LEFT_MARGIN)){
    set_currentmeasurenum (gui, gui->si->leftmeasurenum-1);
    gtk_widget_queue_draw (gui->scorearea);
    return TRUE;
  } 

  if(gui->lefts[line_num] == 0)
    return TRUE;//On an empty system at the bottom where there is not enough room to draw another staff.


  if (event->y < 0)
    get_placement_from_coordinates (&pi, event->x, 0, gui->lefts[line_num],gui->rights[line_num]);
  else
    get_placement_from_coordinates (&pi, event->x, event->y, gui->lefts[line_num],gui->rights[line_num]);
  change_staff(gui->si, pi.staff_number, pi.the_staff);

  if(event->x<LEFT_MARGIN) {
    gint offset = (gint)get_click_height(gui, event->y);
    if(offset<STAFF_HEIGHT/2) {
      if(((DenemoStaff*)gui->si->currentstaff->data)->staff_directives)
	gtk_menu_popup (((DenemoStaff*)gui->si->currentstaff->data)->staffmenu, NULL, NULL, NULL, NULL,0, gtk_get_current_event_time()) ;
    }
    else
      if(((DenemoStaff*)gui->si->currentstaff->data)->voice_directives)
	gtk_menu_popup (((DenemoStaff*)gui->si->currentstaff->data)->voicemenu, NULL, NULL, NULL, NULL,0, gtk_get_current_event_time()) ;
    return TRUE;
  } else if(gui->si->leftmeasurenum==1) {
    if(event->x<KEY_MARGIN-cmajor) {
      popup_menu("/InitialClefEditPopup");
      return TRUE;
    }  else  if(event->x<KEY_MARGIN+key+cmajor) {
      popup_menu("/InitialKeyEditPopup");
      return TRUE;
    } else  if(event->x<KEY_MARGIN+SPACE_FOR_TIME+key) {
      popup_menu("/InitialTimeEditPopup");
      return TRUE;
    }
  }


  if (pi.the_measure != NULL){ /*don't place cursor in a place that is not there*/
    //gui->si->currentstaffnum = pi.staff_number;
    //gui->si->currentstaff = pi.the_staff;
    gui->si->currentmeasurenum = pi.measure_number;
    gui->si->currentmeasure = pi.the_measure;
    gui->si->currentobject = pi.the_obj;
    gui->si->cursor_x = pi.cursor_x;
    gui->si->cursor_appending
      =
      (gui->si->cursor_x ==
       (gint) (g_list_length ((objnode *) gui->si->currentmeasure->data)));
    set_cursor_y_from_click (gui, event->y);
    if(pi.nextmeasure)
      measureright(NULL);

    write_status(gui);


  }


  if(left) {
    // if(!(GDK_CONTROL_MASK&event->state))
      gui->si->markstaffnum = 0;
    //    else
    //  calcmarkboundaries (gui->si);
     lh_down = TRUE;
 
  }
  /* Redraw to show new cursor position, note a real draw is needed because of side effects on display*/
  gtk_widget_draw (gui->scorearea, NULL);
    
  set_cursor_for(event->state | (left?GDK_BUTTON1_MASK:GDK_BUTTON3_MASK));
  perform_command(event->state | (left?GDK_BUTTON1_MASK:GDK_BUTTON3_MASK), GESTURE_PRESS, left);
  
  return TRUE;
}


/**
 * Mouse button release callback 
 *
 */
gint
scorearea_button_release (GtkWidget * widget, GdkEventButton * event)
{
DenemoGUI *gui = Denemo.gui;
 gboolean left = (event->button != 3);

  if(dragging_separator) {
    dragging_separator = FALSE;
    return TRUE;
  }

 //g_signal_handlers_block_by_func(gui->scorearea, G_CALLBACK (scorearea_motion_notify), gui); 
 if(left)
   lh_down = FALSE;
 selecting = FALSE;
 set_cursor_for(event->state&DENEMO_MODIFIER_MASK);
 perform_command(event->state, GESTURE_RELEASE, left);

  return TRUE;
}

gint
scorearea_scroll_event (GtkWidget *widget, GdkEventScroll *event) {
  DenemoGUI *gui = Denemo.gui;
  switch(event->direction) {
    DenemoScriptParam param;
  case GDK_SCROLL_UP:
    if(event->state&GDK_CONTROL_MASK) {
      Denemo.gui->si->zoom *= 1.1;
      displayhelper(gui);
    } else
    if(event->state&GDK_SHIFT_MASK) {
      scroll_left ();

    } else {
      staffup (&param);
      if(!param.status)
	warningmessage("This is the top staff");
    }
    break;
  case GDK_SCROLL_DOWN:
    if(event->state&GDK_CONTROL_MASK) {
      Denemo.gui->si->zoom /= 1.1;
      if(Denemo.gui->si->zoom <0.01)
	Denemo.gui->si->zoom = 0.01;
      displayhelper(gui);
    } else
    if(event->state&GDK_SHIFT_MASK) {
      scroll_right ();
    } else {
    staffdown (&param);
    if(!param.status)
      warningmessage("This is the bottom staff");
    }
    break;
  case GDK_SCROLL_LEFT:
    measureleft (&param);
    if(!param.status)
      warningmessage("This is the first measure");
    break;
  case GDK_SCROLL_RIGHT:
    measureright (&param);
    if(!param.status)
      warningmessage("This is the last measure");
    break;


  }
  displayhelper(Denemo.gui);
  return FALSE;
}
