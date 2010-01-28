/* midi.c
 * functions for direct output to /dev/sequencer
 * and direct input from /dev/midi
 *
 * for Denemo, a gtk+ frontend to GNU Lilypond
 * (c) 2000-2005 Brian Delaney
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "smf.h"

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#else
#define MIDI_NOTEOFF		0x80
#define MIDI_NOTEON		0x90
#define MIDI_KEY_PRESSURE	0xA0

#define MIDI_CTL_CHANGE		0xB0
#define MIDI_PGM_CHANGE		0xC0
#define MIDI_CHN_PRESSURE	0xD0
#define MIDI_PITCH_BEND		0xE0

#define MIDI_SYSTEM_PREFIX	0xF0
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#ifndef G_OS_WIN32
#include <sys/ioctl.h>
#endif
#include <denemo/denemo.h>
#include "draw.h"
#include "audio.h"
#include "jackmidi.h"
#include "instrumentname.h"

#define SEQ_DEV    (Denemo.prefs.sequencer->str)
#define SEQ_DEV_N  0
static int sequencer_fd = -1;
static gboolean shouldremove = FALSE;
static double
midi2hz(int midinum)
{
  double argument = (midinum - 69);
  double expon = (argument / 12);
  return 440 * pow(2, expon);
}

#ifdef HAVE_SYS_SOUNDCARD_H
struct synth_info card_info;

SEQ_DEFINEBUF (128);



static gint ttag;




/* return the time of the last event on the list events */
gdouble get_midi_time(GList *events) {
  smf_event_t *event = g_list_last(events)->data;
return event->time_seconds;
}

DenemoObject *get_obj_for_time(smf_t *smf, gdouble time) {
  if(time<0.0)
    return NULL;
  smf_event_t *event = smf_peek_next_event(smf);
  if(event) {
  gdouble initial = event->time_seconds;
  smf_seek_to_seconds(smf, time);
  event = smf_peek_next_event(smf);
  smf_seek_to_seconds(smf, initial);
  return (DenemoObject *)(event->user_pointer);
  }
  return NULL;
}

/* 
 *  get the midi channel of the currently selected staff
 */
gint get_midi_channel()
{
  gint tracknumber;
  gint channel;
  DenemoStaff *curstaffstruct = (DenemoStaff *) Denemo.gui->si->currentstaff->data;
  if (!strcmp (curstaffstruct->midi_instrument->str, "drums"))
    channel = 9;
  else
    {
      tracknumber = Denemo.gui->si->currentstaffnum-1;
      tracknumber = (tracknumber >= 9) ? tracknumber + 1 : tracknumber;
      channel = tracknumber&0xF;
    }
  return channel ; //staff struct uses encoding 0-15
}

gint get_midi_prognum()
{
  gint prognum;
  DenemoStaff *curstaffstruct = (DenemoStaff *) Denemo.gui->si->currentstaff->data;
  if (curstaffstruct->midi_channel == 9)
    prognum = 0;
  else
    prognum = select_program (curstaffstruct->midi_instrument->str);  
  return prognum;
}

gint get_midi_port()
{
  gint portnumber;
  DenemoStaff *curstaffstruct = (DenemoStaff *) Denemo.gui->si->currentstaff->data;

  portnumber = curstaffstruct->midi_port;
  return portnumber; 
}
/**
 * Dump the global buffer to the sequencer device
 *
 */
void
seqbuf_dump ()
{
  if (_seqbufptr)
    if (write (sequencer_fd, _seqbuf, _seqbufptr) == -1)
      {
	perror (_("Error during seqbuf_dump"));
	//exit (-1);
      }
  _seqbufptr = 0;
}

#endif
/**
 * Close the sequencer device
 *
 */
void
midi_cleanup ()
{
  (void) close (sequencer_fd);
}



static gboolean sequencer_absent = TRUE;
/**
 * Initialise the sequencer device ready for immediate playback
 *
 */
gint
midi_init ()
{
#ifdef HAVE_SYS_SOUNDCARD_H
  if ((sequencer_fd = open (SEQ_DEV, O_WRONLY)) == -1)
    {
      perror (_("Error opening sequencer"));
      return -1;
    }

  card_info.device = 0;

  if (ioctl (sequencer_fd, SNDCTL_SYNTH_INFO, &card_info) == -1)
    {
      perror (_("Cannot get info on soundcard"));
      close (sequencer_fd);
      sequencer_fd = -1;
      return -1;
    }

  printf (_("Synthesizer detected: %s\n"), card_info.name);
  printf (_("Synthesizer supports %d voices.\n"), card_info.nr_voices);

  /* Reset the sequencer */
  if (ioctl (sequencer_fd, SNDCTL_SEQ_RESET) == -1)
    {
      perror (_("Error resetting sequencer"));
      sequencer_fd = -1;
      return -1;
    }
  sequencer_absent = FALSE;
  SEQ_DUMPBUF ();
  close (sequencer_fd);
  sequencer_fd = -1;
#endif
  return 0;
}

void playpitch(double pitch, double duration, double volume, int channel) {
  if(!Denemo.prefs.immediateplayback)
    return;
  play_pitch(pitch, duration, volume, channel);
}

void play_midikey(gint key, double duration, double volume, gint channel){
  if (Denemo.prefs.midi_audio_output == Portaudio)
    playpitch(midi2hz(key), duration, volume, channel);
  else if (Denemo.prefs.midi_audio_output == Jack)
    jack_playpitch(key, 1000 /*duration*/);
  else if (Denemo.prefs.midi_audio_output == Fluidsynth)
    fluid_playpitch(key, 1000 /*duration*/, channel, (int)(0x7f&(int)(volume*127)));
}

/**
 *  Used to play each note in the given chord
 *  (a g_list_foreach function)
 */
static void
playnote (gpointer tone, gpointer chord, int channel)
{
  gint offset;
  gchar key;
  gint voice;
  /* Because mid_c_offset is a measure of notes and we need a measure of
   * half-steps, this array will help */
  const gint key_offset[] = { -10, -8, -7, -5, -3, -1, 0, 2, 4, 5, 7, 9, 11 };

  offset = ((note *) tone)->mid_c_offset;

  /* 60 is middle-C in MIDI keys */
  key = 60 + 12 * (offset / 7) + key_offset[offset % 7 + 6];
  key += ((note *) tone)->enshift;
  voice = g_list_index ((GList *) chord, tone);
  if(sequencer_fd == -1)
    play_midikey(key, 0.3, 0.5/*Denemo.prefs.pcmvolume*/, channel);
#ifdef HAVE_SYS_SOUNDCARD_H
  else {
    SEQ_SET_PATCH (SEQ_DEV_N, voice, channel);
    SEQ_START_NOTE (SEQ_DEV_N, voice, key, 127);
    SEQ_DUMPBUF ();
  }
#endif
}
#ifdef HAVE_SYS_SOUNDCARD_H
/**
 *  Used to stop each tone in the given chord
 *  (a g_list_foreach function)
 */
static void
stoptone (gpointer tone, gpointer chord)
{
  gint offset;
  gint voice;
  gchar key;
  /* Because mid_c_offset is a measure of notes and we need a measure of
   * half-steps, this array will help */
  const gint key_offset[] = { -10, -8, -7, -5, -3, -1, 0, 2, 4, 5, 7, 9, 11 };

  offset = ((note *) tone)->mid_c_offset;

  /* 60 is middle-C in MIDI keys */
  key = 60 + 12 * (offset / 7) + key_offset[offset % 7 + 6];
  key += ((note *) tone)->enshift;
  voice = g_list_index ((GList *) chord, tone);

  SEQ_STOP_NOTE (0, voice, key, 127);
  SEQ_DUMPBUF ();
}

#endif

/**
 * Close the sequencer device.  This is used by playnotes
 *
 */
static gint
close_seqfd (gpointer data)
{
  close (sequencer_fd);
  sequencer_fd = -1;
  shouldremove = FALSE;
  return FALSE;			/* Timeout function won't be called again */
}

/** 
 * This version of the function opens and closes /dev/sequencer w/ each
 * write, as a separate process for performance-type reasons 
 */
void
playnotes (gboolean doit, chord chord_to_play, int channel)
{
  //g_print("playnotes called for channel %d\n", channel);
  if (doit && (sequencer_absent) && chord_to_play.notes) {
    playnote( chord_to_play.notes->data, chord_to_play.notes, channel);
    return;
  }
#ifdef HAVE_SYS_SOUNDCARD_H
  if (doit)
    if (sequencer_fd != -1
	|| (sequencer_fd = open (SEQ_DEV, O_WRONLY)) != -1)
      {
	GList *tone;
	SEQ_START_TIMER ();
	/*
	   g_list_foreach (chord_to_play.notes,
	   (GFunc) playtone, chord_to_play.notes);
	 */
	tone = chord_to_play.notes;
	while (tone)
	  {
	    playnote (tone->data, chord_to_play.notes, channel);
	    tone = tone->next;
	  }
	SEQ_DELTA_TIME (50);


	g_list_foreach (chord_to_play.notes,
			(GFunc) stoptone, chord_to_play.notes);
	if (shouldremove)
	  gtk_timeout_remove (ttag);
	ttag = gtk_timeout_add (1000, close_seqfd, NULL);
	/*shouldremove = TRUE; */
      }
#endif
}

// MIDI input
#include <string.h> /*for memcpy */
#include <math.h>
#include <glib.h>
static  GIOChannel* channel;/* a channel to access /dev/midi by */




static gint *divert_midi_event;
static gint divert_midi_id=0;//id of the DenemoGUI which wants to intercept midi events


#define command ((*buf)&0xFF)
#define notenumber ((*(buf+1))&0xFF)
#define velocity ((*(buf+2))&0xFF)
void process_midi_event(gchar *buf) {
  //g_print("process midi (%s) %x %x %x\n",divert_midi_event?"diverted":"straight", command, notenumber, velocity);
  if(divert_midi_event &&  divert_midi_id==Denemo.gui->id){
    // this is only good for one endianness - FIXME
    *divert_midi_event = 0;//clear 4th byte
    memcpy(divert_midi_event, buf, 3);//midi events are up to three bytes long
    gtk_main_quit();
    return;// not reached
  } 
  if(command==MIDI_NOTEON)
    store_pitch(midi2hz(notenumber));
}

gboolean intercept_midi_event(gint *midi) {
  if(divert_midi_event) {
    infodialog("Recursive midi capture not possible!");/* we could make a stack of them instead ... */
    divert_midi_event = NULL;
    return FALSE;
  }
  divert_midi_event = midi;
  divert_midi_id = Denemo.gui->id;
  gtk_main();
  divert_midi_event = NULL;
  return TRUE;
}
static int
process_callback (GIOChannel *source, GIOCondition condition, gchar * data)
{
  GError *error=NULL;
  gsize bytes_read;
  static gchar buf[3];
  if(channel==NULL)
    return FALSE;//shutdown
  if(channel!=source)
    return FALSE;//shutdown

  //  g_print("Channel %p source %p is %d\n", channel, source, source->is_readable);
  g_io_channel_read_chars (source, buf, 1, &bytes_read, &error);

  if(command==MIDI_SYSTEM_PREFIX) {
    while(command!=0xF7)
      g_io_channel_read_chars (source, buf, 1, &bytes_read, &error);
    return TRUE;
  }
  if(command)
    switch(command) {
    case MIDI_NOTEON:
     
    case MIDI_NOTEOFF:
    case MIDI_KEY_PRESSURE:
    case MIDI_CTL_CHANGE:
    case MIDI_PITCH_BEND:
    case 0xF2:
      {
	g_io_channel_read_chars (source, buf+1, 1, &bytes_read, &error);
	g_io_channel_read_chars (source, buf+2, 1, &bytes_read, &error);            
      }
      if(command==MIDI_NOTEON && velocity==0) {//Zero velocity NOTEON is used as NOTEOFF by some MIDI controllers
	buf[0]=MIDI_NOTEOFF;
	buf[2]=128;
      }
      process_midi_event(buf);
      return TRUE;//means do not remove event source
    case MIDI_PGM_CHANGE:
    case MIDI_CHN_PRESSURE:
    case 0xF3:
      g_io_channel_read_chars (source, buf+1, 1, &bytes_read, &error);
      return TRUE;
    default:
      return TRUE; 
    }
}


gint init_midi_input(void) {
#ifdef _HAVE_JACK_
 return jackmidi_server_running() ? 0 : -1;
#else
#ifdef _HAVE_FLUIDSYNTH_
 return fluid_start_midi_in();
#else
  GError *error = NULL;
  if(!channel)
    channel =  g_io_channel_new_file (Denemo.prefs.midi_in->str,"r", &error);
  if(error)
    return -1;
  g_io_channel_set_encoding       (channel,NULL/* raw binary */,
                                             &error);
  if(error)
    return -2;
  g_io_add_watch_full(channel, G_PRIORITY_HIGH,G_IO_IN|G_IO_PRI, (GIOFunc) process_callback,NULL, NULL);
  //  g_io_add_watch (channel,G_IO_IN, (GIOFunc) process_callback,NULL, NULL);
  return 0;
#endif
#endif
}

gint stop_midi_input(void) {
#ifdef _HAVE_JACK_
  stop_jack();
#else
#ifdef _HAVE_FLUIDSYNTH_
 return fluid_stop_midi_in();
#else
  GError *error = NULL;
  if(channel)
    g_io_channel_shutdown(channel, FALSE, &error);
  if(error)
    g_warning(error->message);
  else
    channel = NULL;
#endif
#endif
  return 0;
}
