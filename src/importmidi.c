/*importmidi.cpp
 * midi file import functions
 * for Denemo, a gtk+ frontend to GNU Lilypond
 * (c) 2003-2005 AJAnderson
 *
 */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <denemo/denemo.h>
#include "importmidi.h"

/* 	TODO
 *  
 *  check to see if memory used by malloc is freed
 *  clean up/optimise code
 *  add functions to add notes to chords
 */

enum newstaffcallbackaction
{ INITIAL, FIRST, LAST, ADDFROMLOAD, BEFORE, AFTER,
	  NEWVOICE,LYRICSTAFF, FIGURESTAFF, CHORDSTAFF
};

void ProcessNoteList(GList *list, midicallback *mididata); 
void addnote(GList *list, midicallback *mididata);
void MeasureCheck(GList *list, midicallback *mididata);

gint
importMidi (gchar * filename, DenemoGUI * gui)
{
  midicallback *mididata = (midicallback *)g_malloc0(sizeof(midicallback));
  mididata->notestack = NULL;
  mididata->chordnotes = NULL;
  mididata->gui = gui;
  mididata->PPQN = 200;
  mididata->bartime = 0;
  mididata->lastoff = 0;
  mididata->track = 0;
  gint ret = 0;			// (-1 on failure)
  gint data = 0;
  FILE *fp = 0;
  gint tracks = 0;

  dnm_deletescore (NULL, gui);

  if (fopen (filename, "r") != 0)
    fp = fopen (filename, "r");

  /*scan to first track */
  
  while (1)
    {
      data = readBytes (fp, 1);
      if (data == 0x4d)
	{
	  data = readBytes (fp, 3);
	  if (data == 0x546864)
	    {
	      tracks = readheader (fp,mididata);
#ifdef DEBUG
      		printf ("\ntracks = %i\n", tracks);
#endif
	    };
	  if (data == 0x54726b)
	    {
#ifdef DEBUG
      	    	printf ("\nReading Track\n");
#endif  
	  readtrack (fp, mididata);
	  tracks--;
	  mididata->track++;
	    };
	};
      if (tracks == 0)
	break;
    };

  fclose (fp);
    
  g_list_foreach(mididata->final_list, (GFunc) addnote, mididata);
  g_free(mididata);
  return ret;
}

DenemoObject * new_dnm_object(notetype length, gint tied){
	DenemoObject *mudela_obj_new;
	tied = (tied > 0) ? tied : length.tied;
	mudela_obj_new = dnm_newchord (length.notetype, length.numofdots, tied);
	return mudela_obj_new;
}

/**
 * Read variable length valuue
 *
 */
gint
readVariable (FILE * fp)
{
  /*so all are 7bit numbers, and all have bit 8 high except the last */
  gint total = 0;
  gint last = 0;
  do
    {
      last = fgetc (fp);
      total = (total << 7) + (last & 0x7f);
    }
  while ((last >> 7) != 0);
  return total;
}

/**
 * Read the number of bytes specified
 *
 * @param fp pointer to the file descriptor
 * @param numb number of bytes to read
 */
gint
readBytes (FILE * fp, gint numb)
{
  gint read = 0;
  while (numb > 0)
    {
      read = (read << 8) + (gint) fgetc (fp);
      numb--;
    }
  return read;
}

/**
 * Read Midi file header
 *
 */
gint
readheader (FILE * fp, midicallback *mididata)
{
  gint header_length;
  gint midi_format;
  gint number_of_tracks;
  gint PPQN;

  header_length = readBytes (fp, 4);
  midi_format = readBytes (fp, 2);
  number_of_tracks = readBytes (fp, 2);
  PPQN = readBytes (fp, 2);
  mididata->PPQN = PPQN;

#ifdef DEBUG
  printf ("\nHeader length confirmed as %d Bytes", header_length);
  printf ("\nMidi format is %d", midi_format);
  printf ("\nNo. of Tracks is %d", number_of_tracks);
  printf ("\nPPQN is %d", PPQN);
#endif
  
  return number_of_tracks;
}

/**
 * Read track from midi file
 *
 */
void
readtrack (FILE * fp, midicallback *mididata)
{
  DenemoScore *si = mididata->gui->si; 
  gint stat;
  gint data1;
  gint data2;
  gint tlength;
  gint time = 0;			/*track time */
  gint dtime;			/*delta time, distance to next event */
  gint event = 0;

  tlength = readBytes (fp, 4);
 
  while (tlength != 0)
    {
      dtime = readVariable (fp);
      time = time + dtime;
      tlength--;
      if (dtime > 127)
	tlength--;		/* 2 digit variable */
      if (dtime > 16383)
	tlength--;		/*  3 digit variable */
      if (dtime > 2097151)
	tlength--;		/*  3 digit variable */
      /* leap of faith, all channels in track ar same */
      stat = readBytes (fp, 1);
      tlength--;
      if (stat == 0xff)
	{
	  /*next Byte is command */
	  data1 = readBytes (fp, 1);
	  /*next Byte is length */
	  data2 = readBytes (fp, 1);
	  /*pretend to read it */
	  switch (data1)
	    {
	    case 88:
	      {
		/*time signature */
		dotimesig (fp, mididata);
		tlength = tlength - 6;
		break;
	      }
	    case 89:
	      {
		/*key signature */
		dokeysig (fp, mididata);
		tlength = tlength - 4;
		break;
	      }
	    case 81:
	      {
		/* set tempo */
		if (data2 == 3)
		  {
		    dotempo (fp, mididata);
		  }
		tlength = tlength - 5;
		break;
	      }
	    case 3:
	      {
		dotrackname (fp, mididata, data2);
		tlength = tlength - (data2 + 2);
		break;
	      }
	    case 4:
	      {
		doinstrname (fp, mididata, data2);
		tlength = tlength - (data2 + 2);
		break;
	      }
	    default:
	      {
		/*read off surplus */
		data1 = readBytes (fp, data2);
		tlength = tlength - (data2 + 2);
		break;
	      }
	    }
	}
      else
	{
	  if (stat < 128)
	    {
	      /*running status event is same as before */
	      data1 = stat;
	    }
	  else
	    {
	      event = stat;
	      data1 = readBytes (fp, 1);
	      tlength--;
	    };

	  switch (event >> 4)
	    {
	    case 8:		/*note off */
	      {
		data2 = readBytes (fp, 1);
		tlength--;
		donoteoff (mididata, (int *) data1, (int *) time);
		break;
	      }
	    case 9:		/*note on */
	      {
		data2 = readBytes (fp, 1);
		tlength--;
		donoteon (mididata, (int *) data1, (int *) data2, (int *) time);
		break;
	      }
	    default:
	      {
		data2 = readBytes (fp, 1);
		tlength--;
		break;
	      }
	    case 11:		/*midi  controller eg. volume, pan */
	      {
		data2 = readBytes (fp, 1);
		//printf("\nMIDI Controller %d, value %d",data1,data2);
		tlength--;
		break;
	      }
	    case 12:		/*change instrument, ignore */
	      {
		//printf("\nChange to Instrument% d",data1);
		break;
	      }

	    };
	}
    }
}

/**
 * Insert time signature into current staff 
 *
 */
void
dotimesig (FILE * fp, midicallback *mididata)
{
  /*only does initial TS */
  DenemoStaff *curstaffstruct = (DenemoStaff *) mididata->gui->si->currentstaff->data;

  curstaffstruct->stime1 = readBytes (fp, 1);
  curstaffstruct->stime2 = (gint) pow (2, (readBytes (fp, 1)));

  mididata->barlength = mididata->PPQN * 4 * curstaffstruct->stime1 / curstaffstruct->stime2;
  readBytes (fp, 2);		/*skip last two characters */
}

/**
 * Insert key signature into the current staff
 *
 */
void
dokeysig (FILE * fp, midicallback *mididata)
{
  /*assume major */
  gint isminor = 0;
  gint key = mididata->key;
  key = readBytes (fp, 1);	/*read in sharps */
  isminor = readBytes (fp, 1);

  if (key > 7)
    key = key - 256;		/*get flat key num, see keysigdialog.cpp */

  DenemoStaff *curstaffstruct = (DenemoStaff *) mididata->gui->si->currentstaff->data;
  curstaffstruct->skey = key;
  curstaffstruct->skey_isminor = isminor;
  dnm_setinitialkeysig (curstaffstruct, key, isminor);
}

void
dotempo (FILE * fp, midicallback *mididata)
{
  gint tempo, n, x;
  x = n = 0;
  while (n++ < 3)
    x = ((x & 0x007FFFFF) << 8) + (gint) readBytes (fp, 1);
  tempo = (gint) (6.0e7 / (double) x);
  mididata->gui->si->tempo = tempo;
}

void
dotrackname (FILE * fp, midicallback *mididata, gint x)
{
  DenemoStaff *curstaffstruct = (DenemoStaff *) mididata->gui->si->currentstaff->data;
  GString *temp = g_string_new(""); 
  while (x-- > 0)
	temp = g_string_append_c(temp, (gchar) readBytes (fp, 1));

  curstaffstruct->denemo_name->str = g_strdup(temp->str);
  dnm_set_lily_name (curstaffstruct->denemo_name, curstaffstruct->lily_name);
  g_string_free (temp, FALSE);
}

void
doinstrname (FILE * fp,  midicallback *mididata, gint x)
{
  DenemoStaff *curstaffstruct = (DenemoStaff *) mididata->gui->si->currentstaff->data;
  GString *temp = g_string_new("");  
  while (x-- > 0)
	temp = g_string_append_c(temp, (gchar) readBytes (fp, 1));

  curstaffstruct->midi_instrument->str = g_strdup(temp->str);
  g_string_free (temp, FALSE);
}

static nstack *
stack (gint *pitch, gint *timeon, gint *duration, gint *measure, gint *staffnum)
{
  nstack *mystack = (nstack *)g_malloc0(sizeof(nstack));
  mystack->pitch = pitch;
  mystack->timeon = timeon;
  return mystack;
}

/**
 * Process note on command 
 */

void
donoteon (midicallback *mididata, gint *pitchon, gint *attack, gint *timeon)
{
  nstack *noteon;
  if (attack == 0)
    {
      donoteoff (mididata, pitchon, timeon);
    }
  else
    {
      /* add a note to the stack */
      noteon = stack(pitchon,timeon,NULL,NULL, NULL);
      mididata->notestack = g_list_append(mididata->notestack, noteon);
#ifdef DEBUG
	      printf ("\npitchon = %i timeon = %i\n", pitchon, timeon);
#endif
    }
}

gint CompareNotes(GList *tmp, gint *pitchoff)
{
        if ((((nstack *) &tmp->data)->pitch == pitchoff) &&
               (((nstack *) &tmp->data)->duration == (int) NULL))
               return 0;
        else
               return 1;
}

void findchordtones(GList *list, midicallback *mididata){
	/*list*/
	gint *l_timeon = ((nstack *) list->data)->timeon;
	gint *l_staffnum = ((nstack *) list->data)->staffnum;
	gint *l_duration = ((nstack *) list->data)->duration;
	gint *l_pitch = ((nstack *) list->data)->pitch;
	/*current note*/
	gint *c_timeon = ((nstack *) mididata->currentnote->data)->timeon;
	gint *c_staffnum = ((nstack *) mididata->currentnote->data)->staffnum;
	gint *c_duration = ((nstack *) mididata->currentnote->data)->duration;
	gint *c_pitch = ((nstack *) mididata->currentnote->data)->pitch;
        //1 2	
	if ((int) l_timeon <= (int) c_timeon < ((int) l_timeon + (int) l_duration)){
	  if ((l_timeon == c_timeon) /*check for notes that belong in the chord*/
		&& (l_duration == c_duration)
		&& (l_staffnum == c_staffnum)
		&& (l_pitch != c_pitch)){
				mididata->chordnotes = g_list_append(mididata->chordnotes, (int *) l_pitch);
	  	printf("\n!!! Chord Tone !!!\n");
	  }
	  else if (((gint) l_timeon == (gint) c_timeon)
			 && (l_pitch != c_pitch) 
			 && ((gint) l_duration != (gint) c_duration)) 
            printf("\n!!! possible voice !!!\n");
	  else if (((gint) l_timeon < (gint) c_timeon)
			 && ((((int) l_timeon + (int) l_duration) > (int) c_timeon)))
		  printf("\n!!! possible voice !!!\n");
	}

}

void findvoicesandchords(GList *list, midicallback *mididata){
	/*list*/
	gint *l_timeon = ((nstack *) list->data)->timeon;
	gint *l_staffnum = ((nstack *) list->data)->staffnum;
	gint *l_duration = ((nstack *) list->data)->duration;
	gint *l_pitch = ((nstack *) list->data)->pitch;
	/*current note*/
	gint *c_timeon = ((nstack *) mididata->currentnote->data)->timeon;
	gint *c_staffnum = ((nstack *) mididata->currentnote->data)->staffnum;
	gint *c_duration = ((nstack *) mididata->currentnote->data)->duration;
	gint *c_pitch = ((nstack *) mididata->currentnote->data)->pitch;
	//0 3 / 1 4
}


/**
 * Process note off command
 */
void
donoteoff (midicallback *mididata, gint *pitchoff, gint *timeoff)
{
  gint duration = 0;
  gint starttime = 0;
  GList *tmp = NULL;
  tmp = g_list_find_custom (mididata->notestack, pitchoff, (GCompareFunc) (int) CompareNotes);
  if (tmp != NULL)
      	starttime = (int) ((nstack *) tmp->data)->timeon;
      	duration = ((gint) timeoff) - starttime;
#ifdef DEBUG
   	g_printf("\nduration = %i\n",duration);
#endif
  	((nstack *) tmp->data)->duration = (int *) duration;
	((nstack *) tmp->data)->measure = (int *) mididata->gui->si->currentmeasurenum;
	((nstack *) tmp->data)->staffnum = (int *) mididata->track;
 	
	mididata->final_list = g_list_append(mididata->final_list, tmp);
	mididata->notestack = g_list_remove (mididata->notestack, tmp);
}


void ProcessNoteList(GList *list, midicallback *mididata) {
	DenemoScore *si = mididata->gui->si;
	gint starttime = (int) ((nstack *) list->data)->timeon;
	gint lastoff = mididata->lastoff;
	gint duration = (int) ((nstack *) list->data)->duration;
	gint currentstaffnum = si->currentstaffnum; 
	gint track = (int) ((nstack *) list->data)->staffnum;
	gint pitch = (int) ((nstack *) list->data)->pitch;
	nstack note;
	DenemoObject *mudela_obj_new;

	restcheck(list,mididata);
        
	while (mididata->chordnotes){
		   mididata->chordnotes = g_list_remove_link(mididata->chordnotes, g_list_first (mididata->chordnotes));
	}
	
	/* call function to return a list of chord tones */
	mididata->currentnote = g_list_copy(list);
	g_list_foreach(mididata->final_list, (GFunc) findchordtones, mididata);

	if ((track + 1) > currentstaffnum) /*if not first track add track */
	  {
	      si->currentstaffnum++;
	      si->currentstaff = g_list_first (si->thescore); /* set the first track to be copied */
	      newstaff (mididata->gui, ADDFROMLOAD, DENEMO_NONE); /* add track */
	      si->currentstaff = g_list_last (si->thescore); /* make last staff the current */
	      mididata->lastoff = 0;
	      si->cursor_x = 0;
	      mididata->bartime = 0;
	      si->currentmeasurenum = 1;
	  }

	if (starttime == mididata->lastoff) {
		notetype length = ConvertLength(duration, mididata);
		mudela_obj_new = new_dnm_object (length,mididata->leftover);
		struct harmonic enote = enharmonic ((int) ((nstack *) list->data)->pitch,mididata->key);
		dnm_addtone (mudela_obj_new, enote.pitch, enote.enshift, mididata->gui->si->cursorclef);
		while (mididata->chordnotes){
		  struct harmonic enote = enharmonic ((int) 
				 mididata->chordnotes->data,mididata->key);
	          dnm_addtone (mudela_obj_new, enote.pitch, enote.enshift, mididata->gui->si->cursorclef);		
		  mididata->chordnotes = g_list_remove_link(mididata->chordnotes, g_list_first (mididata->chordnotes));
		}
		mididata->lastoff = starttime + duration;
		MeasureCheck(list,mididata);
		object_insert (mididata->gui, mudela_obj_new);
		mididata->bartime = mididata->bartime + duration;
	}
	if ((starttime != mididata->lastoff) && (mididata->leftover !=0)){
		mididata->leftover = 0;
		((nstack *) list->data)->timeon = (int *) mididata->lastoff;
		MeasureCheck(list,mididata);
		addnote(list, mididata);
	}
#ifdef DEBUG
	printf("\nNotelist pitch = %i, timeon = %i, duration = %i, staffnum = %i\n", pitch, starttime, duration, track);
#endif
}

void restcheck(GList *tmp, midicallback *mididata){
  gint rest = 0;
  gint PPQN = mididata->PPQN;
  gint starttime = (int) ((nstack *) tmp->data)->timeon;
  gint duration = (int) ((nstack *) tmp->data)->duration;
  DenemoObject *mudela_obj_new;

	if (starttime > mididata->lastoff){
		  rest = starttime - mididata->lastoff;
		      /* Until I find a better way to pad the end of a measure */
		
			  if (rest > PPQN)
			  {
			    if (rest % PPQN){
			      struct notetype length = ConvertLength((rest % PPQN), mididata);
			      mudela_obj_new = new_dnm_object(length,0);
			      object_insert (mididata->gui, mudela_obj_new);
			      mididata->bartime = mididata->bartime + (rest % PPQN);
			      MeasureCheck(tmp,mididata);
			    }
			    
			    rest = rest - (rest % PPQN);
			    while (rest > 0){
			      struct notetype length = ConvertLength(PPQN, mididata);
			      mudela_obj_new = new_dnm_object(length,0);
			      object_insert (mididata->gui, mudela_obj_new);
			      rest = rest - PPQN;
			      mididata->bartime = mididata->bartime + (PPQN);
			      MeasureCheck(tmp,mididata);
			    }
			  }
		          
			  if ((rest < PPQN) && (rest != 0)) {
			    struct notetype length = ConvertLength(rest, mididata);
			    mudela_obj_new = new_dnm_object(length,0);
			    object_insert (mididata->gui, mudela_obj_new);
			    mididata->bartime = mididata->bartime + (rest);
			    MeasureCheck(tmp,mididata);
		      }

		  if(0)// (((starttime - mididata->lastoff) > 0) && (mididata->leftover > 0))
		  {
			mididata->lastoff = mididata->bartime = mididata->barlength;
			mididata->leftover = 0;
			MeasureCheck(tmp,mididata);
		  }
		  else 
			mididata->lastoff = starttime;
	}
}

notetype ConvertLength(gint endnote, midicallback *mididata){
 /*convert length to 2 = crotchet, 3 = quaver etc...... */
  notetype gnotetype;
  gint PPQN = mididata->PPQN;
  gint notetype = 2;
  gint numofdots = 0;
  gint tied = 0;
  gint dsq = (8 * endnote) / PPQN;
  /* 
   *replace this with an algorhthm for replacing this switch?
   */
  switch (dsq)
  {
    case 1:			/*demi-semi-quaver */
      {
	notetype = 5;		/*thirtysecond note*/
	break;
      }
    case 2:			/*semi-quaver */
      {
	notetype = 4;		/*sixteenth note*/
	//numofdots = 1;
	break;
      }
    case 3:			/*dotted semi-quaver */
      {
	notetype = 4;
	//numofdots = 1;
	tied = PPQN / 8;	/*dotted sixteenth*/
	break;
      }
    case 4:			/*quaver */
      {
	notetype = 3;		/*eigth note*/
	break;
      }
    case 5:			/*quaver tied demi */
      {
	notetype = 3;
	tied = PPQN / 8;	
	break;
      }
    case 6:			/*quaver */
      {
	notetype = 3;
	//numofdots = 1;
	tied = PPQN / 4;    /*dotted eigth*/
	break;
      }
    case 7:			/*quaver */
      {
	notetype = 3;
	tied = 3 * PPQN / 8;
	break;
      }
    case 8:			/*crotchet */
      {
	notetype = 2;		/*quarter note*/
	break;
      }
    case 9:			/*crotchet */
      {
	notetype = 2;
	tied = PPQN / 8;
	break;
      }
    case 10:			/*crotchet */
      {
	notetype = 2;
	tied = PPQN / 4;
	break;
      }
    case 11:			/*crotchet */
      {
	notetype = 2;
	tied = PPQN * 3 / 8;
	break;
      }
    case 12:			/*crotchet */
      {
	notetype = 2;	
	//numofdots = 1;
	tied = PPQN / 2;	/*dotted quarter*/
	break;
      }
    case 13:			/*crotchet */
      {
	notetype = 2;
	tied = 5 * PPQN / 8;
	break;
      }
    case 14:			/*crotchet */
      {
	notetype = 2;
	tied = 3 * PPQN / 4;
	break;
      }
    case 15:			/*crotchet */
      {
	notetype = 2;
	tied = 7 * PPQN / 8;
	break;
      }
    case 16:			/*minim */
      {
	notetype = 1;		/*half note*/
	break;
      }
    case 24:
      {
	notetype = 1;		/*dotted half note*/
	//numofdots = 1;
	tied = PPQN;		
	break;
      }
    case 30:
      {
	notetype = 1;
	tied = 7 * PPQN / 4;
	break;
      }
    case 32:			/*semi-breve */
      {
	notetype = 0;		/*whole note*/
	break;
      }
    case 48:
      {
	notetype = 0;		/*dotted whole note*/
	//numofdots = 1;
	tied = PPQN * 2;
	break;
      }
  }
  gnotetype.notetype = notetype;
  gnotetype.numofdots = numofdots;
  gnotetype.tied = tied;
  return gnotetype;
}

/* all this stuff is just checking 
 * to see if we need to add a new measure 
 */
void MeasureCheck(GList *list, midicallback *mididata){
  if (mididata->leftover == 0)
    {
      if (mididata->bartime >= mididata->barlength)	
	{			/* mididata->bartime >= barlenth will be true if there are rests  or notes
				   going over end of measures. */
	  if (!mididata->gui->si->currentmeasure->next)
	    /* Add a measure and make it currentmeasure */
	    mididata->gui->si->currentmeasure =
	      dnm_addmeasures (mididata->gui->si, mididata->gui->si->currentmeasurenum, 1, 1);
	  else
	    mididata->gui->si->currentmeasure = mididata->gui->si->currentmeasure->next;
	  /* Now the stuff that needs to be done for each case */
	  mididata->gui->si->currentmeasurenum++;
	  mididata->gui->si->currentobject = NULL;
	  mididata->gui->si->cursor_x = 0;
	  memcpy (mididata->gui->si->cursoraccs, mididata->gui->si->nextmeasureaccs, SEVENGINTS);
	  memcpy (mididata->gui->si->curmeasureaccs, mididata->gui->si->nextmeasureaccs, SEVENGINTS);
	  mididata->gui->si->curmeasureclef = mididata->gui->si->cursorclef;
	  mididata->bartime = 0;
	}

    }
}

void TiedNoteCheck(GList *list, midicallback *mididata){
  	restcheck(list,mididata);
	gint endnote = (int) ((nstack *) list->data)->duration;
  	gint barlength = (int) mididata->barlength;
  
	if ((mididata->bartime + endnote) > barlength)
	{
	  	mididata->leftover = (int) (barlength - mididata->bartime);	/*tied over bar line */
	  	endnote = endnote - (int) mididata->leftover;
		((nstack *) list->data)->duration = (int *) endnote;
	}
      	else 
		mididata->leftover = (int) 0;
}

/**
 * Add notes to the current staff
 */
void
addnote (GList *list, midicallback *mididata)
{
  /* check to see if notes are tied over */
  TiedNoteCheck(list, mididata);
   
  /* note processing stuff */
  ProcessNoteList(list, mididata);  
}

harmonic
enharmonic (gint input, gint key)
{
  harmonic local;
  local.pitch = (input / 12) - 5;
  local.enshift = input % 12;
  
  switch (local.enshift)
    {
    case 0:			//c
      {
	local.pitch = (key > 6) ? (-1 + local.pitch * 7) : (local.pitch * 7);
	local.enshift = (key > 6) ? (1) : (0);
	break;
      }
    case 1:			//c#
      {
	local.pitch = (key < -3) ? (1 + local.pitch * 7) : (local.pitch * 7);
	local.enshift = (key < -3) ? (-1) : (1);
	break;
      }
    case 2:			//D
      {
	local.pitch = 1 + local.pitch * 7;
	local.enshift = 0;
	break;
      }
    case 3:			//D#
      {
	local.pitch =
	  (key < -1) ? (2 + local.pitch * 7) : (1 + local.pitch * 7);
	local.enshift = (key < -1) ? (-1) : (1);
	break;
      }
    case 4:			//E
      {
	local.pitch =
	  (key < -6) ? (3 + local.pitch * 7) : (2 + local.pitch * 7);
	local.enshift = (key < -6) ? (-1) : (0);
	break;
      }
    case 5:			//F
      {
	local.pitch =
	  (key > 5) ? (2 + local.pitch * 7) : (3 + local.pitch * 7);
	local.enshift = (key > 5) ? (1) : (0);
	break;
      }
    case 6:			//F#
      {
	local.pitch =
	  (key < -4) ? (4 + local.pitch * 7) : (3 + local.pitch * 7);
	local.enshift = (key < -4) ? (-1) : (1);
	break;
      }
    case 7:			//G
      {
	local.pitch = 4 + local.pitch * 7;
	local.enshift = 0;
	break;
      }
    case 8:			//G#
      {
	local.pitch =
	  (key < -2) ? (5 + local.pitch * 7) : (4 + local.pitch * 7);
	local.enshift = (key < -2) ? (-1) : (1);
	break;
      }
    case 9:			//A
      {
	local.pitch = 5 + local.pitch * 7;
	local.enshift = 0;
	break;
      }
    case 10:			//A#
      {
	local.pitch =
	  (key < 0) ? (6 + local.pitch * 7) : (5 + local.pitch * 7);
	local.enshift = (key < 0) ? (-1) : (1);
	break;
      }
    case 11:			//B
      {
	local.pitch =
	  (key < -5) ? (7 + local.pitch * 7) : (6 + local.pitch * 7);
	local.enshift = (key < -5) ? (-1) : (0);
	break;
      }
    };
  return local;
}

