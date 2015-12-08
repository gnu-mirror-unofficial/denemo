
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <errno.h>
#include <math.h>
#include "export/print.h"
#include "core/view.h"
#include "command/scorelayout.h"
#include "command/lilydirectives.h"
#include "export/exportlilypond.h"
#include "scripting/scheme-callbacks.h"
#include <libxml/parser.h>
#include <libxml/tree.h>


static gint changecount = -1;   //changecount when the playback typeset was last created 

typedef struct Timing {
    gdouble time;
    gdouble x;
    gdouble y;
    gint line;
    gint col;
} Timing;

GList *TheTimings = NULL, *LastTiming=NULL, *NextTiming=NULL;
gdouble TheScale = 1.0; //Scale of score font size relative to 18pt
/* Defines for making traversing XML trees easier */

#define FOREACH_CHILD_ELEM(childElem, parentElem) \
for ((childElem) = (parentElem)->xmlChildrenNode; \
     (childElem) != NULL; \
     (childElem) = (childElem)->next)

#define ELEM_NAME_EQ(childElem, childElemName) \
(strcmp ((gchar *)(childElem)->name, (childElemName)) == 0)

#define ILLEGAL_ELEM(parentElemName, childElem) \
do \
  { \
    g_warning ("Illegal element inside <%s>: <%s>", parentElemName, \
               (childElem)->name); \
  } while (0)

#define RETURN_IF_ELEM_NOT_FOUND(parentElemName, childElem, childElemName) \
do \
  { \
    if (childElem == NULL) \
      { \
        g_warning ("Element <%s> not found inside <%s>", childElemName, \
                   parentElemName); \
        return -1; \
      } \
  } while (0)

/**
 * Get the text from the child node list of elem, convert it to an integer,
 * and return it.  If unsuccessful, return G_MAXINT.
 */
static gint
getXMLIntChild (xmlNodePtr elem)
{
  gchar *text = (gchar *) xmlNodeListGetString (elem->doc, elem->xmlChildrenNode, 1);
  gint num = G_MAXINT;
  if (text == NULL)
    {
      g_warning ("No child text found %s", elem->name);
    }
  else
    {
      if (sscanf (text, " %d", &num) != 1)
        {
          g_warning ("Could not convert child text \"%s\" of <%s> to number", text, elem->name);
          num = G_MAXINT;
        }
      g_free (text);
    }
  return num;
}





//Ensures the playback view window is visible.
static void
show_playback_view (void)
{
    GtkWidget *w = gtk_widget_get_toplevel (Denemo.playbackview);
    if (!gtk_widget_get_visible (w))
        activate_action ("/MainMenu/ViewMenu/" TogglePlaybackView_STRING);
    else
        gtk_window_present (GTK_WINDOW (w));
}
        

//draw a circle 
static void
place_spot (cairo_t * cr, gint x, gint y)
{
  cairo_move_to (cr, x, y);
  cairo_arc (cr, x, y, PRINTMARKER / 4, 0.0, 2 * M_PI);
  cairo_fill (cr);
}

static void
get_window_size (gint * w, gint * h)
{
  GdkWindow *window;
  if (!GTK_IS_LAYOUT (Denemo.playbackview))
    window = gtk_widget_get_window (GTK_WIDGET (Denemo.playbackview));
  else
    window = gtk_layout_get_bin_window (GTK_LAYOUT (Denemo.playbackview));
  if (window)
    {
     
#if GTK_MAJOR_VERSION==2
      gdk_drawable_get_size (window, w, h);
#else
      *w = gdk_window_get_width (window);
      *h = gdk_window_get_height (window);
#endif
    }
}

static void
get_window_position (gint * x, gint * y)
{
    
#if 0
  GtkAdjustment *adjust = gtk_range_get_adjustment (GTK_RANGE (Denemo.printhscrollbar));
  *x = (gint) gtk_adjustment_get_value (adjust);
  adjust = gtk_range_get_adjustment (GTK_RANGE (Denemo.printvscrollbar));
  *y = gtk_adjustment_get_value (adjust);
#else
  *x = *y = 0; //g_warning("Not calculating window scroll effects");
#endif
}


//over-draw the evince widget with padding etc ...
static gboolean
overdraw_print (cairo_t * cr)
{
  gint x, y;
  gdouble last, next;
  if (!Denemo.project->movement->playingnow)
    return TRUE;
  if (TheTimings == NULL)
        return TRUE;
  if (TheTimings->next == NULL)
        return TRUE;
  if (LastTiming == NULL)
        {
            LastTiming = TheTimings;
            NextTiming = TheTimings->next;
        }        
  cairo_scale (cr, 5.61*TheScale, 5.61*TheScale);
  cairo_set_source_rgba (cr, 0.9, 0.5, 1.0, 0.3);

    gdouble time = Denemo.project->movement->playhead;
    GList *g;
    last = ((Timing *)LastTiming->data)->time;
    next = ((Timing *)NextTiming->data)->time;

    if (time<last)
        {
            LastTiming = TheTimings;
            NextTiming = TheTimings->next;
        }
    last = ((Timing *)LastTiming->data)->time;
    next = ((Timing *)NextTiming->data)->time;    
        
    for(g=LastTiming;g && g->next;g=g->next)
        {
            
            LastTiming = g;
            last = ((Timing *)g->data)->time;
            NextTiming = g->next;
            next = ((Timing *)g->next->data)->time;
           //g_print (" %f between %f %f time greater is %f %d so ",  time,  last, next, time - next + 0.01, (time > (next -0.01)));
            if((time > (last -0.01)) && !(time > (next -0.01)))
                    {//g_print ("draw for %.2f\n", last);
                        cairo_rectangle (cr, ((Timing *)((LastTiming)->data))->x  - (PRINTMARKER/5)/4, ((Timing *)((LastTiming)->data))->y - (PRINTMARKER/5)/2, PRINTMARKER/5, PRINTMARKER/5);
                        cairo_fill (cr);
                        return TRUE;
                    }

        }
    if(NextTiming)
        {
        cairo_rectangle (cr, ((Timing *)((NextTiming)->data))->x  - (PRINTMARKER/5)/4, ((Timing *)((NextTiming)->data))->y - (PRINTMARKER/5)/2, PRINTMARKER/5, PRINTMARKER/5);
        cairo_fill (cr);
        }        
  return TRUE;
}
static gboolean
predraw_print (cairo_t * cr)
{
  gint x, y, width, height;
  get_window_size (&width, &height);
  get_window_position (&x, &y);
  cairo_translate (cr, -x, -y);
  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 0, 0.0, (gdouble)width, (gdouble)height);
  cairo_fill (cr);
  return FALSE;//propagate further
}
#if GTK_MAJOR_VERSION==3
static gint
playbackview_draw_event (G_GNUC_UNUSED GtkWidget * w, cairo_t * cr)
{
  return overdraw_print (cr);
}
static gint
playbackview_predraw_event (G_GNUC_UNUSED GtkWidget * w, cairo_t * cr)
{
  return predraw_print (cr);
}
#else
static gint
playbackview_draw_event (GtkWidget * widget, GdkEventExpose * event)
{
  /* Setup a cairo context for rendering and clip to the exposed region. */
  cairo_t *cr = gdk_cairo_create (event->window);
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);
  overdraw_print (cr);
  cairo_destroy (cr);
  return TRUE;
}
static gint
playbackview_predraw_event (GtkWidget * widget, GdkEventExpose * event)
{
  /* Setup a cairo context for rendering and clip to the exposed region. */
  cairo_t *cr = gdk_cairo_create (event->window);
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);
  predraw_print (cr);
  cairo_destroy (cr);
  return FALSE; //propagate further
}
#endif

static Timing *get_svg_position(gchar *id, GList *ids)
{
  for(;ids;ids=ids->next)
        {//g_print ("Testing %s with %s\n", ids->data, id);
            if (g_str_has_prefix ((gchar*)ids->data, id))
                {
                    
                  Timing *timing = (Timing *)g_malloc (sizeof(Timing));
                  if (2==sscanf ((gchar*)ids->data, "Note-%*d-%*d translate(%lf,%lf)%*s%*s", &timing->x, &timing->y))
                    {
                        g_print ("Found Position %.2f %.2f\n", timing->x, timing->y);
                        return timing;
                    } else if (2==sscanf ((gchar*)ids->data, "Rest-%*d-%*d translate(%lf,%lf)%*s%*s", &timing->x, &timing->y))
                    {
                        g_print ("Found Position %.2f %.2f\n", timing->x, timing->y);
                        return timing;
                    }
                    
                }
            
            
        }
    g_warning ("Failed to find a position in events.txt for %s\n", id);
    return NULL;
}

static void add_note (Timing *t)
{
    TheTimings = g_list_append (TheTimings, (gpointer)t);
    g_print ("Added %.2f seconds (%.2f,%.2f)\n", t->time, t->x, t->y);
}
static void free_timings (void)
{
    GList *g;
    for (g = TheTimings; g;g=g->next)
        {
            g_free(g->data);
        }
    g_list_free (TheTimings);
    TheTimings = NULL;
    LastTiming = NextTiming = NULL;
}

static void compute_timings (gchar *base, GList *ids)
{
    free_timings();
    gchar *events = g_build_filename (base, "events.txt", NULL);
    FILE *fp = fopen (events, "r");  
    g_print ("Collected %d ids\n", g_list_length (ids));
    if(fp)
        {
            gdouble moment, duration;
            gchar type [10];
            gint duration_type, col, line, midi;
            gdouble tempo = 60;
            gdouble timeCoef =  4;
            gdouble latestMoment = 0;
            gdouble adjustedElapsedTime = 0;
            gdouble nextTempo = 0;
            gdouble nextTempoMoment = 0;
            gboolean incomingTempo = FALSE;
            while (2 == fscanf (fp, "%lf%10s", &moment, type)) 
                {
                 g_print ("moment %.2f %s latestMoment %.2f\n", moment, type, latestMoment);
                  if (!strcmp (type, "tempo"))  
                        {
                            if (1 == fscanf (fp, "%lf", &nextTempo))
                                {
                                nextTempoMoment = moment;//g_print ("Next %s %.2f\n", type, nextTempo);
                                nextTempo = nextTempo / 4;
                                incomingTempo = TRUE;
                                } else g_warning ("Malformed events file");
                        }
                 else
                    {
                        if (!strcmp (type, "note"))
                            {
                            if (4 == fscanf (fp, "%*s%lf%*s%d%d%d", &duration, &col, &line, &midi))
                                    {
                                       // g_print ("moment ... %.2f %s %d %.2f %d %d %d\n", moment, type, duration_type, duration, col, line, midi); 
                                       if (incomingTempo)
                                        {
                                            if (moment > nextTempoMoment)
                                                {
                                                    tempo = nextTempo;//g_print (" tempo %.2f\n", tempo);
                                                    timeCoef = (60 / tempo) * 4;//g_print (" timeCoef %.2f\n", timeCoef);
                                                    incomingTempo = FALSE;
                                                }
                                        }
                                        gdouble elapsedTime = moment - latestMoment;
                                        adjustedElapsedTime += elapsedTime * timeCoef;//g_print ("adjustedElapsedtime %f\n", adjustedElapsedTime);
                                        gchar *idStr;
                                        Timing *timing;

                                                idStr = g_strdup_printf ("Note-%d-%d" , line, col);
                                                timing = get_svg_position (idStr, ids);
                                                
                                                if(timing)
                                                    {
                                                    timing->line = line;
                                                    timing->col = col;
                                                    timing->time = adjustedElapsedTime;
                                                    add_note (timing);g_print ("AdjustedElapsed time %.2f note %d\n", adjustedElapsedTime, midi);
                                                    }
                                                
                                    }
                                    else
                                    g_warning ("Could not parse type %s\n", type);
                            }            
                                    
                        else if(!strcmp (type, "rest"))
                            {
                                if (3 == fscanf (fp, "%*s%lf%*s%d%d",  &duration, &col, &line))
                                    {
                                       // g_print ("moment ... %.2f %s %d %.2f %d %d %d\n", moment, type, duration_type, duration, col, line, midi); 
                                       if (incomingTempo)
                                        {
                                            if (moment > nextTempoMoment)
                                                {
                                                    tempo = nextTempo;//g_print (" tempo %.2f\n", tempo);
                                                    timeCoef = (60 / tempo) * 4;//g_print (" timeCoef %.2f\n", timeCoef);
                                                    incomingTempo = FALSE;
                                                }
                                        }
                                        gdouble elapsedTime = moment - latestMoment;
                                        adjustedElapsedTime += elapsedTime * timeCoef;//g_print ("adjustedElapsedtime %f\n", adjustedElapsedTime);
                                        gchar *idStr;
                                        Timing *timing;                                        
                                            
                                            
                                            
                                            idStr = g_strdup_printf ("Rest-%d-%d" , line, col);
                                            timing = get_svg_position (idStr, ids);
                                            if(timing)
                                                {
                                                timing->line = line;
                                                timing->col = col;
                                                timing->time = adjustedElapsedTime;
                                                add_note (timing);g_print ("AdjustedElapsed time %.2f rest \n", adjustedElapsedTime);
                                                }
                                            
                                } //rest
                            else g_warning ("Don't know how to handle %s\n", type);
                            }
                            latestMoment = moment;
                        }// not tempo
                    } //while events
                fclose (fp);
            } //if events file
                    
    g_free (events); 
}

static GList * create_positions (gchar *filename)
{
  GList *ret = NULL;
  GError *err = NULL;
  xmlDocPtr doc = NULL;
  xmlNsPtr ns;
  xmlNodePtr rootElem;
  /* ignore blanks between nodes that appear as "text" */
  xmlKeepBlanksDefault (0);
  /* Try to parse the file. */
  doc = xmlParseFile (filename);
  if (doc == NULL)
    {
      g_warning ("Could not read svg file %s", filename);
     
    }
    else
    {
      rootElem = xmlDocGetRootElement (doc);
      xmlNodePtr childElem;
      FOREACH_CHILD_ELEM (childElem, rootElem)
      {
          if (ELEM_NAME_EQ (childElem, "g"))
            { xmlNodePtr grandChildElem;
              gchar *id = xmlGetProp (childElem, (xmlChar *) "id");
                 FOREACH_CHILD_ELEM (grandChildElem, childElem)
                   {
                     if (ELEM_NAME_EQ (grandChildElem, "g"))   //grouping to set color to black
                      { xmlNodePtr greatgrandChildElem;
                        FOREACH_CHILD_ELEM (greatgrandChildElem, grandChildElem)
                            {
                                if (ELEM_NAME_EQ (greatgrandChildElem, "path"))
                                    {
                                        gchar *coords = xmlGetProp (greatgrandChildElem, (xmlChar *) "transform");
                                        g_print ("ID %s has Coords %s\n", id, coords);
                                        if (id && coords)
                                            {
                                            gchar *data = g_strconcat (id, coords, NULL);
                                            ret = g_list_append (ret, data);
                                            xmlFree (id);
                                            xmlFree (coords);
                                            }
                                    } else g_warning ("Found %s", greatgrandChildElem->name);
                                }
                        }
                    }
                }
        }
    }
    g_print ("Read %d ids from file %s\n", g_list_length (ret), filename);
  return ret;  
}
static void
set_playback_view (void)
{
  GFile *file;
  gchar *filename = g_strdup (get_print_status()->printname_svg[get_print_status()->cycle]);
  g_print("Output to %s\n", filename);
    if ((get_print_status()->invalid == 0) && !(g_file_test (filename, G_FILE_TEST_EXISTS))){
      {
          g_free (filename);
          filename = g_strconcat (get_print_status()->printbasename[get_print_status()->cycle], "-page-2.svg", NULL);
          g_print ("Failed, skipping title page to %s", filename);
          get_print_status()->invalid = (g_file_test (filename, G_FILE_TEST_EXISTS)) ? 0 : 3;
      }
    }

    if (get_print_status()->invalid == 0) 
    get_print_status()->invalid = (g_file_test (filename, G_FILE_TEST_EXISTS)) ? 0 : 3;


 if (get_print_status()->invalid == 0)
    {
     
      compute_timings (g_path_get_dirname(filename), create_positions (filename)); 
      if(Denemo.playbackview)
        gtk_image_set_from_file (GTK_IMAGE (Denemo.playbackview), filename);
      else
        Denemo.playbackview = gtk_image_new_from_file (filename);

      static gboolean shown_once = FALSE;   //Make sure the user knows that the printarea is on screen
      if (!shown_once)
        {
          shown_once = TRUE;
          show_playback_view ();
        }
    }
    else g_warning ("get print status invalid %d\n", get_print_status()->invalid);
    g_free (filename);
  return;
}

static void
playbackview_finished (G_GNUC_UNUSED GPid pid, G_GNUC_UNUSED gint status, gboolean print)
{
  progressbar_stop ();

  g_spawn_close_pid (get_print_status()->printpid);
  g_print ("background %d\n", get_print_status()->background);
  if (get_print_status()->background == STATE_NONE)
    {
      call_out_to_guile ("(FinalizeTypesetting)");
      process_lilypond_errors ((gchar *) get_printfile_pathbasename ());
    }
  else
    {
      if (LilyPond_stderr != -1)
        close (LilyPond_stderr);
      LilyPond_stderr = -1;
    }
  get_print_status()->printpid = GPID_NONE;
  set_playback_view ();
  
  changecount = Denemo.project->changecount;
 //FIXME LOAD get_printfile_pathbasename ().midi into libsmf so that we play the LilyPond generated MIDI.
  
 // start_normal_cursor ();

 //  if (Denemo.playbackview)
 //   {
 //    GtkWidget* printarea = gtk_widget_get_toplevel (Denemo.playbackview);
  //   if (gtk_window_is_active (GTK_WINDOW (printarea)))
 //       gtk_window_present (GTK_WINDOW (printarea));
 //   }
}



static gboolean
initialize_typesetting (void)
{
  return call_out_to_guile ("(InitializeTypesetting)");
}

//A button could be placed in the playback view to create an svg file from the view...
static void
copy_svg (void)
{
  gchar *filename;
  gchar *outuri = get_output_uri_from_scoreblock ();
  gchar *outpath;
  gchar *outname;
  outuri += strlen ("file://"); //skip the uri bit of it
  outpath = g_path_get_dirname (outuri);
  outname = g_path_get_basename (outuri);
  GtkWidget *chooser = gtk_file_chooser_dialog_new (_("SVG creation"),
                                                    GTK_WINDOW (Denemo.window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    GTK_STOCK_CANCEL,
                                                    GTK_RESPONSE_REJECT,
                                                    GTK_STOCK_SAVE,
                                                    GTK_RESPONSE_ACCEPT, NULL);
  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name (filter, _("SVG files"));
  gtk_file_filter_add_pattern (filter, "*.svg");
  gtk_file_filter_add_pattern (filter, "*.SVG");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), outpath);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), outname);
  gtk_widget_show_all (chooser);
  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
  else
    filename = NULL;
  gtk_widget_destroy (chooser);

  if (filename)
    {
      gchar *contents;
      gsize length;
    
        
      if (g_file_get_contents (get_print_status()->printname_svg[get_print_status()->cycle], &contents, &length, NULL))
        {
            
            if ((!g_file_test (filename, G_FILE_TEST_EXISTS)) || confirm (_( "SVG creation"), _( "File Exists, overwrite?")))  
                {          
                  if (!g_file_set_contents (filename, contents, length, NULL))
                    {
                      gchar *msg = g_strdup_printf (_("Errno %d:\nCould not copy %s to %s. Perhaps because some other process is using the destination file. Try again with a new location\n"),
                                                    errno,
                                                    get_print_status()->printname_svg[get_print_status()->cycle],
                                                    filename);
                      warningdialog (msg);
                      g_free (msg);
                    }
                  else
                    {
                      gchar *uri = g_strconcat ("file://", filename, NULL);
                      if (strcmp(uri, get_output_uri_from_scoreblock ()))
                        score_status (Denemo.project, TRUE);
                      set_current_scoreblock_uri (uri);
                     
                      //g_print ("I have copied %s to %s (default was %s) uri %s\n", get_print_status()->printname_svg[get_print_status()->cycle], filename, outname, uri);
                    }
                  g_free (contents);
                }
        }
      g_free (outpath);
      g_free (outname);
      g_free (filename);
    }
}





//re-creates the svg image and displays it
static void remake_playback_view ()
{
    if (Denemo.project->movement->markstaffnum)
        Denemo.project->movement->markstaffnum = 0;//It can (and would otherwise) typeset just the selection - would that be useful?
    create_svg (FALSE, FALSE);//there is a typeset() function defined which does initialize_typesetting() ...
    g_child_watch_add (get_print_status()->printpid, (GChildWatchFunc) playbackview_finished, (gpointer) (FALSE));
}

//returns TRUE if a re-build has been kicked off.
static gboolean update_playback_view (void)
{
    g_print ("Testing %d not equal %d \n", changecount, Denemo.project->changecount);
 if (changecount != Denemo.project->changecount)
        {
        call_out_to_guile ("(d-PlaybackView)");//this installs the temporary directives to typeset svg and then
        return TRUE;
        }
return FALSE;
}
//Typeset and svg and display in playbackview window. Scale is the font size relative to 18.0 pt.
void
display_svg (gdouble scale)
{
    TheScale = scale; 
    (void)remake_playback_view ();
      //bring print view back to show cursor
    if (Denemo.textview)
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (Denemo.textview),
                                      gtk_text_buffer_get_insert (Denemo.textbuffer),
                                      0.0,
                               TRUE, 0.5, 0.5);
}


static gint Locationx, Locationy;
static void find_object (GtkWidget *event_box, GdkEventButton *event)
{
    gint x = event->x;
    gint y = event->y;
    g_print ("At %d %d\n", x, y);
    GList *g;

    for (g = TheTimings; g;g=g->next)
        {
            Timing *timing = g->data;
            if((x-timing->x*5.61*TheScale < PRINTMARKER/(2)) && (y-timing->y*5.61*TheScale < PRINTMARKER/(2)))
                {
                    g_print ("Found line %d column %d\n", timing->line, timing->col);
                    Locationx = timing->col;
                    Locationy = timing->line;
                    goto_lilypond_position (timing->line, timing->col);
                    call_out_to_guile ("(DenemoSetPlaybackStart)");
                    g_print ("Set Playback Start %d column %d\n", timing->line, timing->col);
                    return;
                    
                }
            //g_print ("compare %d %d with %.2f, %.2f\n", x, y, timing->x*5.61*TheScale, timing->y*5.61*TheScale);
        }                    
}
static void start_play (GtkWidget *event_box, GdkEventButton *event)
{
    gint x = event->x;
    gint y = event->y;
    //g_print ("At %d %d\n", x, y);
    if (update_playback_view ())
        {
            infodialog (_("Please wait while the Playback View is re-typeset"));
            return;
        }
    GList *g;
    for (g = TheTimings; g;g=g->next)
        {
            Timing *timing = g->data;
            if((x-timing->x*5.61*TheScale < PRINTMARKER/(2)) && (y-timing->y*5.61*TheScale < PRINTMARKER/(2)))
                {
                    
                    if ((timing->col == Locationx) && (timing->line == Locationy))
                        {
                            call_out_to_guile ("(d-DenemoPlayCursorToEnd)");
                            g_print ("Found same line %d column %d\n", timing->line, timing->col);
                        }
                    else
                        {
                            goto_lilypond_position (timing->line, timing->col);
                            call_out_to_guile ("(if (not (d-NextChord)) (d-MoveCursorRight))(DenemoSetPlaybackEnd)");
                            g_print ("Set playback end to %d column %d\n", timing->line, timing->col);
                            call_out_to_guile ("(d-OneShotTimer 500 \"(d-Play)\")");
                        }
                    
                    return;
                    
                }
           // g_print ("compare %d %d with %.2f, %.2f\n", x, y, timing->x*5.61*TheScale, timing->y*5.61*TheScale);
        }                    
}


static gint
hide_playback_on_delete (void)
{
  activate_action ("/MainMenu/ViewMenu/" TogglePlaybackView_STRING);
  return TRUE;
}

static void play_button (void)
{
   if (update_playback_view ())
        {
            infodialog (_("Please wait while the Playback View is re-typeset"));
            return;
        }
    call_out_to_guile ("(d-Play)");
}
void
install_svgview (GtkWidget * top_vbox)
{
  if (Denemo.playbackview)
        return;
        
        
  GtkWidget *main_vbox = gtk_vbox_new (FALSE, 1);
  GtkWidget *main_hbox = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), main_hbox, FALSE, TRUE, 0);
  GtkWidget *hbox;
  hbox = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);

  GtkWidget *button = (GtkWidget*)gtk_button_new_with_label (_("Play"));
  g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (play_button), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  button = (GtkWidget*)gtk_button_new_with_label (_("Stop"));
  g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (call_out_to_guile), "(d-Stop)");
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  
  if (top_vbox == NULL)
    top_vbox = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  // if(!Denemo.prefs.manualtypeset)
  //      gtk_window_set_urgency_hint (GTK_WINDOW(Denemo.window), TRUE);//gtk_window_set_transient_for (GTK_WINDOW(top_vbox), GTK_WINDOW(Denemo.window));
  gtk_window_set_title (GTK_WINDOW (top_vbox), _("Denemo Playback View"));
  gtk_window_set_default_size (GTK_WINDOW (top_vbox), 600, 750);
  //g_signal_connect (G_OBJECT (top_vbox), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
  g_signal_connect (G_OBJECT (top_vbox), "delete-event", G_CALLBACK (hide_playback_on_delete), NULL);
  gtk_container_add (GTK_CONTAINER (top_vbox), main_vbox);
  GtkWidget *score_and_scroll_hbox = gtk_scrolled_window_new (NULL, NULL);
 

  gtk_box_pack_start (GTK_BOX (main_vbox), score_and_scroll_hbox, TRUE, TRUE, 0);
  

  
  
  gchar *filename = get_print_status()->printname_svg[get_print_status()->cycle];
  Denemo.playbackview = (GtkWidget *) gtk_image_new_from_file (filename);
    // gtk_container_add (GTK_CONTAINER (score_and_scroll_hbox), Denemo.playbackview);
    //instead use an hbox to prevent the GtkImage widget expanding beyond the image size, which then causes positioning errors.
    hbox = gtk_hbox_new (FALSE, 1);
    gtk_container_add (GTK_CONTAINER (score_and_scroll_hbox), hbox);
    GtkWidget *event_box = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (hbox), event_box, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (event_box), Denemo.playbackview);
    g_signal_connect (G_OBJECT (event_box), "button_press_event", G_CALLBACK (find_object), NULL);
    g_signal_connect (G_OBJECT (event_box), "button_release_event", G_CALLBACK (start_play), NULL);

  //  gtk_box_pack_start (GTK_BOX (hbox), Denemo.playbackview, FALSE, FALSE, 0);
  
  
  
  if (Denemo.prefs.newbie)
    gtk_widget_set_tooltip_markup (score_and_scroll_hbox,
                                   _("This window shows the typeset score as one long page. During playback the notes playing are highlighted"));
#if GTK_MAJOR_VERSION != 2
  g_signal_connect_after (G_OBJECT (Denemo.playbackview), "draw", G_CALLBACK (playbackview_draw_event), NULL);
  g_signal_connect (G_OBJECT (Denemo.playbackview), "draw", G_CALLBACK (playbackview_predraw_event), NULL);
#else
  g_signal_connect_after (G_OBJECT (Denemo.playbackview), "expose_event", G_CALLBACK (playbackview_draw_event), NULL);
  g_signal_connect (G_OBJECT (Denemo.playbackview), "expose_event", G_CALLBACK (playbackview_predraw_event), NULL);
#endif
  gtk_widget_show_all (main_vbox);
  gtk_widget_hide (top_vbox);
}
