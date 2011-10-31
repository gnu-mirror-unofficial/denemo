/* changenotehead.c
 * Changes the type of notehead if required
 * 
 * for Denemo, a gtk+ frontend to GNU Lilypond
 * (c) Adam Tee 2000-2005
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calculatepositions.h"
#include "contexts.h"
#include "dialogs.h"
#include "draw.h"
#include "objops.h"
#include "staffops.h"
#include "utils.h"
#include "commandfuncs.h"
/**
 * Array of different Notehead types
 */
gchar *notehead[4] = { N_("Normal"), N_("Cross"), N_("Diamond"),
  N_("Harmonic")
};


/**
 * Set the correct enum value for the selected 
 * notehead
 */
enum headtype
texttohead (gchar * text)
{
  if (g_strcasecmp (text, _("Normal")) == 0)
    return DENEMO_NORMAL_NOTEHEAD;
  else if (g_strcasecmp (text, _("Cross")) == 0)
    return DENEMO_CROSS_NOTEHEAD;
  else if (g_strcasecmp (text, _("Diamond")) == 0)
    return DENEMO_DIAMOND_NOTEHEAD;
  else if (g_strcasecmp (text, _("Harmonic")) == 0)
    return DENEMO_HARMONIC_NOTEHEAD;
  else
    return DENEMO_NORMAL_NOTEHEAD;
}


/**
 * Set current notes notehead to the selected 
 * value
 */
void
insertnotehead (DenemoScore * si, gchar * notehead_string)
{
  DenemoObject *obj = (DenemoObject *)
    (si->currentobject ? si->currentobject->data : NULL);


  if (obj != NULL && obj->type == CHORD)
    {
      /* Lilypond's behavior is a bit anomalous here. It doesn't seem
       * to like giving chords non-standard noteheads. This is
       * just a default behavior for the time being. */
      ((note *) ((chord *) obj->object)->notes->data)->noteheadtype =
	texttohead (notehead_string);
    }

}

/**
 * Notehead selection dialog
 * Displays the notehead type in a Combobox
 * Callback - insert_notehead
 */
void
set_notehead (GtkAction *action, gpointer param)
{
  DenemoGUI *gui = Denemo.gui;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *combo;
  GtkWidget *content_area;
  gint i;
  if(!action) {
    if(  ((DenemoScriptParam *)param)->string && ((DenemoScriptParam *)param)->string->len) {
      insertnotehead (gui->si, ((DenemoScriptParam *)param)->string->str);
      ((DenemoScriptParam *)param)->status = TRUE;
      return;
    } else {
      if(param)
	((DenemoScriptParam *)param)->status = FALSE;
      return;
    }
  }
    
  dialog =
    gtk_dialog_new_with_buttons (_("Change Notehead"),
				 GTK_WINDOW (Denemo.window),
				 (GtkDialogFlags) (GTK_DIALOG_MODAL |
						   GTK_DIALOG_DESTROY_WITH_PARENT),
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 GTK_STOCK_CANCEL, GTK_STOCK_CANCEL, NULL);


  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  label = gtk_label_new (_("Select Notehead Type"));
  gtk_container_add (GTK_CONTAINER (content_area), label);

  combo = gtk_combo_box_text_new ();
  for(i=0;i<G_N_ELEMENTS(notehead);i++)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combo), notehead[i]);

  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
  
  gtk_container_add (GTK_CONTAINER (content_area), combo);

  gtk_widget_grab_focus (combo);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
  
  gtk_widget_show_all (dialog);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      gint num =
        gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
       insertnotehead (gui->si, notehead[num]);
    }

  g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);  
  gtk_widget_destroy(dialog);
  displayhelper (gui);
}
