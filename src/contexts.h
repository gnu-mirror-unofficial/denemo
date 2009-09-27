/* contexts.h
 * Context finders: functions that find the current clef, key, and time
 * signature contexts for the initial(?) measures being displayed 
 * Also, set the initial values for the staffs from the first measure data
 *
 * for Denemo, a gtk+ frontend to GNU Lilypond
 * (c) 1999-2005 Matthew Hiller
 */

#ifndef __CONTEXTS_H__
#define __CONTEXTS_H__
#include <denemo/denemo.h>

void find_leftmost_staffcontext (DenemoStaff * curstaffstruct,
				 DenemoScore *si);

void find_leftmost_allcontexts (DenemoScore *si);
gint 
find_prevailing_clef(DenemoScore *si);
DenemoObject *
get_clef_before_object(objnode *curobj);
#endif /* __CONTEXTS_H__ */
