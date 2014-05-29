#ifndef __SCHEME_CALLBACKS__
#define __SCHEME_CALLBACKS__

#include <libguile.h>
#include <denemo/denemo.h>

#define DENEMO_SCHEME_PREFIX "d-"

#define MODELESS_STRING "Modeless"
#define CLASSICMODE_STRING "ClassicMode"
#define INSERTMODE_STRING "InsertMode"
#define EDITMODE_STRING "EditMode"
#define NOTE_E_STRING "Note"
#define REST_E_STRING "Rest"
#define BLANK_E_STRING "Blank"
#define RHYTHM_E_STRING "Rhythm"
#define ToggleToolbar_STRING "ToggleToolbar"
#define TogglePlaybackControls_STRING "TogglePlaybackToolbar"
#define ToggleMidiInControls_STRING "ToggleMidiInToolbar"
#define ToggleRhythmToolbar_STRING "ToggleRhythmToolbar"
#define ToggleActionMenu_STRING  "ToggleActionMenu"
#define ToggleObjectMenu_STRING  "ToggleObjectMenu"
#define ToggleLilyText_STRING  "ToggleLilyText"
#define ToggleScript_STRING  "ToggleScript"
#define TogglePrintView_STRING  "TogglePrintView"
#define ToggleScoreLayout_STRING  "ToggleScoreLayout"
#define ToggleCommandManager_STRING  "ToggleCommandManager"
#define ToggleLyricsView_STRING  "ToggleLyricsView"
#define ToggleConsoleView_STRING  "ToggleConsoleView"
#define ToggleScoreView_STRING  "ToggleScoreView"
#define ToggleScoreTitles_STRING  "ToggleScoreTitles"
#define QuickEdits_STRING  "QuickEdits"
#define RecordScript_STRING  "RecordScript"
#define ReadOnly_STRING  "ReadOnly"

typedef void (*callback_function) (GtkAction*, DenemoScriptParam*);

SCM scheme_call_callback (SCM optional, callback_function callback);

void update_scheme_snippet_ids (void);

SCM scheme_hide_menus (SCM);
SCM scheme_hide_buttons (SCM);
SCM scheme_destroy_buttons (void);
SCM scheme_hide_window (SCM);
SCM scheme_script_callback (SCM script, SCM params);
SCM scheme_get_option (SCM);
SCM scheme_get_text_selection (void);
SCM scheme_get_padding (void);
SCM scheme_get_relative_font_size (void);
SCM scheme_initialize_script (SCM);
SCM scheme_load_command (SCM);
SCM scheme_activate_menu_item (SCM);
SCM scheme_locate_dotdenemo (SCM);
SCM scheme_get_type (SCM);
SCM scheme_get_lilypond (SCM);
SCM scheme_get_tuplet (SCM);
SCM scheme_set_tuplet (SCM);
SCM scheme_set_background (SCM);
SCM scheme_get_clip_obj_type (SCM m, SCM n);
SCM scheme_get_clip_objects (SCM);
SCM scheme_put_clip_obj (SCM tag, SCM value);
SCM scheme_clear_clipboard (SCM);
SCM scheme_get_staffs_in_clipboard (SCM);
SCM scheme_get_measures_in_staff (SCM);
SCM scheme_get_staffs_in_movement (SCM);
SCM scheme_staff_to_voice (SCM);
SCM scheme_voice_to_staff (SCM);
SCM scheme_is_voice (void);
SCM scheme_adjust_xes (SCM);
SCM scheme_highlight_cursor (SCM);
SCM scheme_get_nonprinting (SCM);
SCM scheme_set_nonprinting (SCM);
SCM scheme_is_grace (SCM);
SCM scheme_is_tied (SCM);
SCM scheme_is_slur_start (SCM);
SCM scheme_is_slur_end (SCM);
SCM scheme_is_cresc_start (SCM);
SCM scheme_is_cresc_end (SCM);
SCM scheme_is_dim_start (SCM);
SCM scheme_is_dim_end (SCM);
SCM scheme_is_in_selection (void);
SCM scheme_is_appending (void);
SCM scheme_shift_cursor (SCM);
SCM scheme_get_movement (void);
SCM scheme_get_staff (void);
SCM scheme_get_measure (void);
SCM scheme_set_object_display_width (SCM);
SCM scheme_get_horizontal_position (void);
SCM scheme_get_cursor_note (SCM);
SCM scheme_get_cursor_note_with_octave (SCM);
SCM scheme_get_editing_time (void);
SCM scheme_debug_object (SCM);
SCM scheme_destroy_scheme_init (void);
SCM scheme_get_note_name (SCM);
SCM scheme_insert_rest (SCM);
SCM scheme_put_whole_measure_rests (void);
SCM scheme_get_note (SCM);
SCM scheme_get_note_from_top (SCM);
SCM scheme_get_note_from_top_as_midi (SCM);
SCM scheme_get_notes (SCM);
SCM scheme_get_note_at_cursor (void);
SCM scheme_get_dots (void);
SCM scheme_get_note_base_duration (void);
SCM scheme_get_note_duration (void);
SCM scheme_get_onset_time (void);
SCM scheme_get_recorded_midi_tempo (SCM index);
SCM scheme_get_imported_midi_track (SCM index);
SCM scheme_delete_imported_midi (void);
SCM scheme_midi_record (SCM script);
SCM scheme_get_current_midi_track (void);
SCM scheme_get_imported_midi_tracks (void);
SCM scheme_get_recorded_midi_duration (void);
SCM scheme_set_duration_in_ticks (SCM);
SCM scheme_get_duration_in_ticks (void);
SCM scheme_get_base_duration_in_ticks (void);
SCM scheme_get_end_tick (void);
SCM scheme_get_start_tick (void);
SCM scheme_get_measure_number (void);
SCM scheme_cursor_to_nth_note_height (SCM number);
SCM scheme_cursor_to_next_note_height (void);
SCM scheme_cursor_to_note (SCM);
SCM scheme_get_prevailing_keysig (SCM);
SCM scheme_get_prevailing_timesig (SCM);
SCM scheme_get_prevailing_clef (SCM);
SCM scheme_get_prevailing_clef_as_lilypond (SCM);
SCM scheme_get_prevailing_keysig_as_lilypond (SCM);
SCM scheme_get_prevailing_timesig_as_lilypond (SCM);
SCM scheme_get_prevailing_duration (SCM);
SCM scheme_set_prevailing_keysig (SCM);
SCM scheme_increment_initial_keysig (SCM);
SCM scheme_increment_keysig (SCM);
SCM scheme_add_movement (SCM);
SCM scheme_change_chord_notes (SCM);
SCM scheme_put_note_name (SCM);
SCM scheme_set_accidental (SCM);
SCM scheme_put_rest (SCM);
SCM scheme_insert_note_in_chord (SCM);
SCM scheme_diatonic_shift (SCM);
SCM scheme_next_object (void);
SCM scheme_prev_object (void);
SCM scheme_next_object_in_measure (void);
SCM scheme_prev_object_in_measure (void);
SCM scheme_next_selected_object (SCM);
SCM scheme_prev_selected_object (SCM);
SCM scheme_next_chord (SCM);
SCM scheme_prev_chord (SCM);
SCM scheme_next_chord_in_measure (SCM);
SCM scheme_prev_chord_in_measure (SCM);
SCM scheme_next_note (SCM);
SCM scheme_prev_note (SCM);
SCM scheme_create_snippet_from_object (SCM);
SCM scheme_select_snippet (SCM);
SCM scheme_insert_snippet (SCM);
SCM scheme_next_standalone_directive (SCM);
SCM scheme_prev_standalone_directive (SCM);
SCM scheme_next_standalone_directive_in_measure (SCM);
SCM scheme_prev_standalone_directive_in_measure (SCM);
SCM scheme_chordize (SCM);
SCM scheme_set_prefs (SCM);
SCM scheme_get_boolean_pref (SCM);
SCM scheme_get_int_pref (SCM);
SCM scheme_get_string_pref (SCM);
SCM scheme_attach_quit_callback (SCM);
SCM scheme_detach_quit_callback (void);
SCM scheme_get_input_source (void);
SCM scheme_popup_menu (SCM);
SCM scheme_get_target_info (void);
SCM scheme_get_new_target (void);
SCM scheme_get_new_point (void);
SCM scheme_get_reference_point (void);
SCM scheme_get_offset (void);
SCM scheme_get_control_point (SCM);
SCM scheme_get_curve (void);
SCM scheme_get_positions (SCM);
SCM scheme_http (SCM hname, SCM page, SCM other, SCM poststr);
SCM scheme_goto_position (SCM movement, SCM staff, SCM measure, SCM object);
SCM scheme_create_palette_button (SCM palette, SCM lbl, SCM tltp, SCM scrp);
SCM scheme_set_palette_shape (SCM palette, SCM horizontal, SCM limit);
SCM scheme_show_palettes (SCM);
SCM scheme_get_user_input (SCM label, SCM prompt, SCM init, SCM modal);
SCM scheme_get_user_input_with_snippets (SCM label, SCM prompt, SCM init, SCM modal);
SCM scheme_warningdialog (SCM);
SCM scheme_infodialog (SCM);
SCM scheme_progressbar (SCM);
SCM scheme_progressbar_stop (void);
SCM scheme_typeset_for_script (SCM);
SCM scheme_print_typeset_pdf (void);
SCM scheme_get_char (void);
SCM scheme_get_keypress (void);
SCM scheme_get_command_keypress (void);
SCM scheme_get_command (void);
SCM scheme_get_command_from_user (void);
SCM scheme_lock_directive (SCM);
SCM scheme_set_action_script_for_tag (SCM tag, SCM script);
SCM scheme_put_standalone_directive (SCM tag, SCM width);
SCM scheme_directive_change_tag (SCM tag);
SCM scheme_text_edit_standalone_directive (SCM);
SCM scheme_delete_object_directive (SCM);
SCM scheme_put_text_clipboard (SCM);
SCM scheme_get_username (void);
SCM scheme_get_password (void);
SCM scheme_get_keyboard_state (void);
SCM scheme_set_midi_thru (SCM);
SCM scheme_get_recorded_midi_on_tick (void);
SCM scheme_get_note_for_midi_key (SCM);
SCM scheme_get_recorded_midi_note (void);
SCM scheme_rewind_recorded_midi (void);
SCM scheme_get_midi (void);
SCM scheme_set_midi_capture (SCM);
SCM scheme_toggle_playalong (void);
SCM scheme_toggle_conduct (void);
SCM scheme_compute_midi_note_durations (void);
SCM scheme_get_marked_midi_note (void);
SCM scheme_get_marked_midi_note_seconds (void);
SCM scheme_mark_first_note_onset (void);
SCM scheme_advance_marked_midi (SCM amount);
SCM scheme_insert_marked_midi_note (void);

SCM scheme_create_timebase (SCM);
SCM scheme_put_midi (SCM);
SCM scheme_output_midi (SCM);
SCM scheme_output_midi_bytes (SCM);
SCM scheme_play_midikey (SCM);
SCM scheme_pending_midi (SCM);
SCM scheme_play_midi_note (SCM note, SCM volume, SCM channel, SCM duration);
SCM scheme_one_shot_timer (SCM duration_amount, SCM callback);
SCM scheme_timer (SCM duration_amount, SCM callback);
SCM scheme_kill_timer (SCM);
SCM scheme_has_figures (SCM);
SCM scheme_bass_figure (SCM bass, SCM harmony);
SCM scheme_spell_check_midi_chord (SCM);
SCM scheme_get_cursor_note_as_midi (SCM);
SCM scheme_get_note_as_midi (void);
SCM scheme_refresh_display (SCM);
SCM scheme_set_saved (SCM);
SCM scheme_get_saved (SCM);
SCM scheme_mark_status (SCM);
SCM scheme_get_help (SCM);
SCM scheme_load_keybindings (SCM);
SCM scheme_save_keybindings (SCM);
SCM scheme_clear_keybindings (SCM);
SCM scheme_load_commandset (SCM);
SCM scheme_zoom (SCM);
SCM scheme_master_tempo (SCM);
SCM scheme_movement_tempo (SCM);
SCM scheme_master_volume (SCM);
SCM scheme_staff_master_volume (SCM);
SCM scheme_set_enharmonic_position (SCM);
SCM scheme_get_midi_tuning (void);
SCM scheme_get_flattest (void);
SCM scheme_get_sharpest (void);
SCM scheme_get_temperament (void);
SCM scheme_rewind_midi (SCM);
SCM scheme_next_midi_notes (SCM);
SCM scheme_restart_play (void);
SCM scheme_get_midi_on_time (void);
SCM scheme_get_midi_off_time (void);
SCM scheme_midi_in_append_edit (void);
SCM scheme_midi_in_checking (void);
SCM scheme_midi_in_listening (void);
SCM scheme_set_playback_interval (SCM start, SCM end);
SCM scheme_adjust_playback_start (SCM);
SCM scheme_adjust_playback_end (SCM);
SCM scheme_user_screenshot (SCM);
SCM scheme_delete_screenshot (SCM);
SCM scheme_push_clipboard (SCM);
SCM scheme_pop_clipboard (SCM);
SCM scheme_delete_selection (SCM);
SCM scheme_set_thumbnail_selection (SCM);
SCM scheme_create_thumbnail (SCM, SCM);
SCM scheme_exit (SCM);
SCM scheme_take_snapshot (SCM);
SCM scheme_select_default_layout (void);
SCM scheme_create_layout (SCM name, SCM force);
SCM scheme_get_layout_id (void);
SCM scheme_get_current_staff_layout_id (void);
SCM scheme_select_layout_id (SCM);
SCM scheme_lilypond_for_part (void);
SCM scheme_typeset_part (void);
SCM scheme_reduce_layout_to_lilypond (void);
SCM scheme_get_layout_name (void);
SCM scheme_select_next_layout (void);
SCM scheme_select_first_layout (void);
SCM scheme_select_next_custom_layout (void);
SCM scheme_select_first_custom_layout (void);
SCM scheme_open_source (SCM);
SCM scheme_export_recorded_audio (void);
SCM scheme_open_source_file (SCM);
SCM scheme_open_source_audio_file (SCM);
SCM scheme_close_source_audio (SCM);
SCM scheme_start_audio_play (SCM);
SCM scheme_stop_audio_play (SCM);
SCM scheme_set_audio_lead_in (SCM);
SCM scheme_audio_is_playing (void);
SCM scheme_next_audio_timing (SCM);
SCM scheme_increase_guard (SCM);
SCM scheme_decrease_guard (SCM);
SCM scheme_undo (SCM);
SCM scheme_new_window (SCM);
SCM scheme_stage_for_undo (SCM);
SCM scheme_get_last_change (SCM);
SCM scheme_get_menu_path (SCM);
SCM scheme_get_checksum (SCM);
SCM scheme_set_newbie (SCM);
SCM scheme_synchronize_lyric_cursor (void);
SCM scheme_insert_text_in_verse (SCM);
SCM scheme_get_verse (SCM);
SCM scheme_put_verse (SCM);
SCM scheme_append_to_verse (SCM);
SCM scheme_get_id (SCM);
SCM scheme_add_keybinding (SCM command, SCM binding);
SCM scheme_get_label (SCM);
SCM scheme_get_menu_position (SCM);
SCM scheme_get_lily_version (SCM);
SCM scheme_check_lily_version (SCM);
SCM scheme_input_filter_names (SCM);
SCM scheme_write_status (SCM);

SCM scheme_object_directive_get_tag (SCM);
SCM scheme_standalone_directive_get_tag (SCM);
SCM scheme_chord_directive_get_tag (SCM);
SCM scheme_note_directive_get_tag (SCM);
SCM scheme_staff_directive_get_tag (SCM);
SCM scheme_voice_directive_get_tag (SCM);
SCM scheme_score_directive_get_tag (SCM);
SCM scheme_clef_directive_get_tag (SCM);
SCM scheme_timesig_directive_get_tag (SCM);
SCM scheme_tuplet_directive_get_tag (SCM);
SCM scheme_stemdirective_directive_get_tag (SCM);
SCM scheme_keysig_directive_get_tag (SCM);
SCM scheme_scoreheader_directive_get_tag (SCM);
SCM scheme_header_directive_get_tag (SCM);
SCM scheme_paper_directive_get_tag (SCM);
SCM scheme_layout_directive_get_tag (SCM);
SCM scheme_movementcontrol_directive_get_tag (SCM);

SCM scheme_chord_directive_get_nth_tag (SCM);
SCM scheme_note_directive_get_nth_tag (SCM);
SCM scheme_staff_directive_get_nth_tag (SCM);
SCM scheme_voice_directive_get_nth_tag (SCM);
SCM scheme_score_directive_get_nth_tag (SCM);
SCM scheme_clef_directive_get_nth_tag (SCM);
SCM scheme_timesig_directive_get_nth_tag (SCM);
SCM scheme_tuplet_directive_get_nth_tag (SCM);
SCM scheme_stemdirective_directive_get_nth_tag (SCM);
SCM scheme_keysig_directive_get_nth_tag (SCM);
SCM scheme_scoreheader_directive_get_nth_tag (SCM);
SCM scheme_header_directive_get_nth_tag (SCM);
SCM scheme_paper_directive_get_nth_tag (SCM);
SCM scheme_layout_directive_get_nth_tag (SCM);
SCM scheme_movementcontrol_directive_get_nth_tag (SCM);

SCM scheme_directive_get_nth_tag_strict_note (SCM);
SCM scheme_directive_get_for_tag_strict_note (SCM);


SCM scheme_delete_note_directive (SCM);
SCM scheme_activate_note_directive (SCM);
SCM scheme_text_edit_note_directive (SCM);

SCM scheme_delete_chord_directive (SCM);
SCM scheme_activate_chord_directive (SCM);
SCM scheme_text_edit_chord_directive (SCM);

SCM scheme_delete_staff_directive (SCM);
SCM scheme_activate_staff_directive (SCM);
SCM scheme_text_edit_staff_directive (SCM);

SCM scheme_delete_voice_directive (SCM);
SCM scheme_activate_voice_directive (SCM);
SCM scheme_text_edit_voice_directive (SCM);

SCM scheme_delete_score_directive (SCM);
SCM scheme_activate_score_directive (SCM);
SCM scheme_text_edit_score_directive (SCM);

SCM scheme_delete_clef_directive (SCM);
SCM scheme_activate_clef_directive (SCM);
SCM scheme_text_edit_clef_directive (SCM);

SCM scheme_delete_timesig_directive (SCM);
SCM scheme_activate_timesig_directive (SCM);
SCM scheme_text_edit_timesig_directive (SCM);

SCM scheme_delete_tuplet_directive (SCM);
SCM scheme_activate_tuplet_directive (SCM);
SCM scheme_text_edit_tuplet_directive (SCM);

SCM scheme_delete_stemdirective_directive (SCM);
SCM scheme_activate_stemdirective_directive (SCM);
SCM scheme_text_edit_stemdirective_directive (SCM);

SCM scheme_delete_keysig_directive (SCM);
SCM scheme_activate_keysig_directive (SCM);
SCM scheme_text_edit_keysig_directive (SCM);

SCM scheme_delete_scoreheader_directive (SCM);
SCM scheme_activate_scoreheader_directive (SCM);
SCM scheme_text_edit_scoreheader_directive (SCM);

SCM scheme_delete_header_directive (SCM);
SCM scheme_activate_header_directive (SCM);
SCM scheme_text_edit_header_directive (SCM);

SCM scheme_delete_paper_directive (SCM);
SCM scheme_activate_paper_directive (SCM);
SCM scheme_text_edit_paper_directive (SCM);

SCM scheme_delete_layout_directive (SCM);
SCM scheme_activate_layout_directive (SCM);
SCM scheme_text_edit_layout_directive (SCM);

SCM scheme_delete_movementcontrol_directive (SCM);
SCM scheme_activate_movementcontrol_directive (SCM);
SCM scheme_text_edit_movementcontrol_directive (SCM);


SCM scheme_object_directive_get_minpixels (SCM);
SCM scheme_object_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_standalone_directive_get_minpixels (SCM);
SCM scheme_chord_directive_get_minpixels (SCM);
SCM scheme_note_directive_get_minpixels (SCM);
SCM scheme_staff_directive_get_minpixels (SCM);
SCM scheme_voice_directive_get_minpixels (SCM);
SCM scheme_score_directive_get_minpixels (SCM);
SCM scheme_clef_directive_get_minpixels (SCM);
SCM scheme_timesig_directive_get_minpixels (SCM);
SCM scheme_tuplet_directive_get_minpixels (SCM);
SCM scheme_stemdirective_directive_get_minpixels (SCM);
SCM scheme_keysig_directive_get_minpixels (SCM);
SCM scheme_scoreheader_directive_get_minpixels (SCM);
SCM scheme_header_directive_get_minpixels (SCM);
SCM scheme_paper_directive_get_minpixels (SCM);
SCM scheme_layout_directive_get_minpixels (SCM);
SCM scheme_movementcontrol_directive_get_minpixels (SCM);
SCM scheme_standalone_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_chord_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_note_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_staff_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_voice_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_score_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_clef_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_timesig_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_keysig_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_header_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_paper_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_layout_directive_put_minpixels (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_minpixels (SCM tag, SCM value);

//cloned from minpixels


SCM scheme_object_directive_get_data (SCM);
SCM scheme_object_directive_put_data (SCM tag, SCM value);
SCM scheme_standalone_directive_get_data (SCM);
SCM scheme_chord_directive_get_data (SCM);
SCM scheme_note_directive_get_data (SCM);
SCM scheme_staff_directive_get_data (SCM);
SCM scheme_voice_directive_get_data (SCM);
SCM scheme_score_directive_get_data (SCM);
SCM scheme_clef_directive_get_data (SCM);
SCM scheme_timesig_directive_get_data (SCM);
SCM scheme_tuplet_directive_get_data (SCM);
SCM scheme_stemdirective_directive_get_data (SCM);
SCM scheme_keysig_directive_get_data (SCM);
SCM scheme_scoreheader_directive_get_data (SCM);
SCM scheme_header_directive_get_data (SCM);
SCM scheme_paper_directive_get_data (SCM);
SCM scheme_layout_directive_get_data (SCM);
SCM scheme_movementcontrol_directive_get_data (SCM);
SCM scheme_standalone_directive_put_data (SCM tag, SCM value);
SCM scheme_chord_directive_put_data (SCM tag, SCM value);
SCM scheme_note_directive_put_data (SCM tag, SCM value);
SCM scheme_staff_directive_put_data (SCM tag, SCM value);
SCM scheme_voice_directive_put_data (SCM tag, SCM value);
SCM scheme_score_directive_put_data (SCM tag, SCM value);
SCM scheme_clef_directive_put_data (SCM tag, SCM value);
SCM scheme_timesig_directive_put_data (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_data (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_data (SCM tag, SCM value);
SCM scheme_keysig_directive_put_data (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_data (SCM tag, SCM value);
SCM scheme_header_directive_put_data (SCM tag, SCM value);
SCM scheme_paper_directive_put_data (SCM tag, SCM value);
SCM scheme_layout_directive_put_data (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_data (SCM tag, SCM value);





SCM scheme_standalone_directive_get_grob (SCM);
SCM scheme_chord_directive_get_grob (SCM);
SCM scheme_note_directive_get_grob (SCM);
SCM scheme_staff_directive_get_grob (SCM);
SCM scheme_voice_directive_get_grob (SCM);
SCM scheme_score_directive_get_grob (SCM);
SCM scheme_clef_directive_get_grob (SCM);
SCM scheme_timesig_directive_get_grob (SCM);
SCM scheme_tuplet_directive_get_grob (SCM);
SCM scheme_stemdirective_directive_get_grob (SCM);
SCM scheme_keysig_directive_get_grob (SCM);
SCM scheme_standalone_directive_get_grob (SCM);
SCM scheme_standalone_directive_put_grob (SCM tag, SCM value);
SCM scheme_chord_directive_put_grob (SCM tag, SCM value);
SCM scheme_note_directive_put_grob (SCM tag, SCM value);
SCM scheme_score_directive_put_grob (SCM tag, SCM value);
SCM scheme_clef_directive_put_grob (SCM tag, SCM value);
SCM scheme_timesig_directive_put_grob (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_grob (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_grob (SCM tag, SCM value);
SCM scheme_keysig_directive_put_grob (SCM tag, SCM value);





SCM scheme_standalone_directive_get_midibytes (SCM);
SCM scheme_chord_directive_get_midibytes (SCM);
SCM scheme_note_directive_get_midibytes (SCM);
SCM scheme_staff_directive_get_midibytes (SCM);
SCM scheme_voice_directive_get_midibytes (SCM);
SCM scheme_score_directive_get_midibytes (SCM);
SCM scheme_movementcontrol_directive_get_midibytes (SCM);
SCM scheme_standalone_directive_put_midibytes (SCM tag, SCM value);
SCM scheme_chord_directive_put_midibytes (SCM tag, SCM value);
SCM scheme_note_directive_put_midibytes (SCM tag, SCM value);
SCM scheme_staff_directive_put_midibytes (SCM tag, SCM value);
SCM scheme_voice_directive_put_midibytes (SCM tag, SCM value);
SCM scheme_score_directive_put_midibytes (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_midibytes (SCM tag, SCM value);
SCM scheme_standalone_directive_get_override (SCM);
SCM scheme_chord_directive_get_override (SCM);
SCM scheme_note_directive_get_override (SCM);
SCM scheme_staff_directive_get_override (SCM);
SCM scheme_voice_directive_get_override (SCM);
SCM scheme_score_directive_get_override (SCM);
SCM scheme_standalone_directive_put_override (SCM tag, SCM value);
SCM scheme_chord_directive_put_override (SCM tag, SCM value);
SCM scheme_note_directive_put_override (SCM tag, SCM value);
SCM scheme_staff_directive_put_override (SCM tag, SCM value);
SCM scheme_voice_directive_put_override (SCM tag, SCM value);
SCM scheme_score_directive_put_override (SCM tag, SCM value);
SCM scheme_note_directive_put_graphic (SCM tag, SCM value);
SCM scheme_chord_directive_put_graphic (SCM tag, SCM value);
SCM scheme_standalone_directive_put_graphic (SCM tag, SCM value);
SCM scheme_staff_directive_put_graphic (SCM tag, SCM value);
SCM scheme_voice_directive_put_graphic (SCM tag, SCM value);
SCM scheme_score_directive_put_graphic (SCM tag, SCM value);
SCM scheme_chord_directive_put_display (SCM tag, SCM value);
SCM scheme_chord_directive_put_prefix (SCM tag, SCM value);
SCM scheme_chord_directive_put_postfix (SCM tag, SCM value);
SCM scheme_chord_directive_get_display (SCM);
SCM scheme_chord_directive_get_prefix (SCM);
SCM scheme_chord_directive_get_postfix (SCM);
SCM scheme_note_directive_put_display (SCM tag, SCM value);
SCM scheme_note_directive_put_prefix (SCM tag, SCM value);
SCM scheme_note_directive_put_postfix (SCM tag, SCM value);
SCM scheme_note_directive_get_display (SCM);
SCM scheme_note_directive_get_prefix (SCM);
SCM scheme_note_directive_get_postfix (SCM);
SCM scheme_standalone_directive_put_display (SCM tag, SCM value);
SCM scheme_standalone_directive_put_prefix (SCM tag, SCM value);
SCM scheme_standalone_directive_put_postfix (SCM tag, SCM value);
SCM scheme_standalone_directive_get_display (SCM);
SCM scheme_standalone_directive_get_prefix (SCM);
SCM scheme_standalone_directive_get_postfix (SCM);
SCM scheme_staff_directive_put_display (SCM tag, SCM value);
SCM scheme_staff_directive_put_prefix (SCM tag, SCM value);
SCM scheme_staff_directive_put_postfix (SCM tag, SCM value);
SCM scheme_staff_directive_get_display (SCM);
SCM scheme_staff_directive_get_prefix (SCM);
SCM scheme_staff_directive_get_postfix (SCM);
SCM scheme_voice_directive_put_display (SCM tag, SCM value);
SCM scheme_voice_directive_put_prefix (SCM tag, SCM value);
SCM scheme_voice_directive_put_postfix (SCM tag, SCM value);
SCM scheme_voice_directive_get_display (SCM);
SCM scheme_voice_directive_get_prefix (SCM);
SCM scheme_voice_directive_get_postfix (SCM);
SCM scheme_score_directive_put_display (SCM tag, SCM value);
SCM scheme_score_directive_put_prefix (SCM tag, SCM value);
SCM scheme_score_directive_put_postfix (SCM tag, SCM value);
SCM scheme_score_directive_get_display (SCM);
SCM scheme_score_directive_get_prefix (SCM);
SCM scheme_score_directive_get_postfix (SCM);
SCM scheme_score_directive_get_width (SCM);
SCM scheme_score_directive_get_height (SCM);
SCM scheme_score_directive_get_x (SCM);
SCM scheme_score_directive_get_gx (SCM);
SCM scheme_score_directive_get_tx (SCM);
SCM scheme_score_directive_put_x (SCM tag, SCM value);
SCM scheme_score_directive_put_gx (SCM tag, SCM value);
SCM scheme_score_directive_put_tx (SCM tag, SCM value);
SCM scheme_score_directive_get_y (SCM);
SCM scheme_score_directive_get_gy (SCM);
SCM scheme_score_directive_get_ty (SCM);
SCM scheme_score_directive_put_y (SCM tag, SCM value);
SCM scheme_score_directive_put_gy (SCM tag, SCM value);
SCM scheme_score_directive_put_ty (SCM tag, SCM value);
SCM scheme_note_directive_put_x (SCM tag, SCM value);
SCM scheme_note_directive_get_x (SCM);
SCM scheme_chord_directive_put_x (SCM tag, SCM value);
SCM scheme_chord_directive_get_x (SCM);
SCM scheme_note_directive_put_y (SCM tag, SCM value);
SCM scheme_note_directive_get_y (SCM);
SCM scheme_chord_directive_put_y (SCM tag, SCM value);
SCM scheme_chord_directive_get_y (SCM);
SCM scheme_note_directive_put_tx (SCM tag, SCM value);
SCM scheme_note_directive_get_tx (SCM);
SCM scheme_chord_directive_put_tx (SCM tag, SCM value);
SCM scheme_chord_directive_get_tx (SCM);
SCM scheme_note_directive_put_ty (SCM tag, SCM value);
SCM scheme_note_directive_get_ty (SCM);
SCM scheme_chord_directive_put_ty (SCM tag, SCM value);
SCM scheme_chord_directive_get_ty (SCM);
SCM scheme_note_directive_put_gx (SCM tag, SCM value);
SCM scheme_note_directive_get_gx (SCM);
SCM scheme_chord_directive_put_gx (SCM tag, SCM value);
SCM scheme_chord_directive_get_gx (SCM);
SCM scheme_note_directive_put_gy (SCM tag, SCM value);
SCM scheme_note_directive_get_gy (SCM);
SCM scheme_chord_directive_put_gy (SCM tag, SCM value);
SCM scheme_chord_directive_get_gy (SCM);
SCM scheme_standalone_directive_put_x (SCM tag, SCM value);
SCM scheme_standalone_directive_get_x (SCM);
SCM scheme_standalone_directive_put_y (SCM tag, SCM value);
SCM scheme_standalone_directive_get_y (SCM);
SCM scheme_standalone_directive_put_tx (SCM tag, SCM value);
SCM scheme_standalone_directive_get_tx (SCM);
SCM scheme_standalone_directive_put_ty (SCM tag, SCM value);
SCM scheme_standalone_directive_get_ty (SCM);
SCM scheme_standalone_directive_put_gx (SCM tag, SCM value);
SCM scheme_standalone_directive_get_gx (SCM);
SCM scheme_standalone_directive_put_gy (SCM tag, SCM value);
SCM scheme_standalone_directive_get_gy (SCM);
SCM scheme_note_directive_get_width (SCM);
SCM scheme_chord_directive_get_width (SCM);
SCM scheme_standalone_directive_get_width (SCM);
SCM scheme_note_directive_get_height (SCM);
SCM scheme_chord_directive_get_height (SCM);
SCM scheme_standalone_directive_get_height (SCM);
SCM scheme_clef_directive_put_display (SCM tag, SCM value);
SCM scheme_clef_directive_put_prefix (SCM tag, SCM value);
SCM scheme_clef_directive_put_postfix (SCM tag, SCM value);
SCM scheme_clef_directive_put_graphic (SCM tag, SCM value);
SCM scheme_clef_directive_get_display (SCM);
SCM scheme_clef_directive_get_prefix (SCM);
SCM scheme_clef_directive_get_postfix (SCM);
SCM scheme_clef_directive_put_x (SCM tag, SCM value);
SCM scheme_clef_directive_put_y (SCM tag, SCM value);
SCM scheme_clef_directive_put_tx (SCM tag, SCM value);
SCM scheme_clef_directive_put_ty (SCM tag, SCM value);
SCM scheme_clef_directive_put_gx (SCM tag, SCM value);
SCM scheme_clef_directive_put_gy (SCM tag, SCM value);
SCM scheme_clef_directive_put_override (SCM tag, SCM value);
SCM scheme_clef_directive_get_x (SCM);
SCM scheme_clef_directive_get_y (SCM);
SCM scheme_clef_directive_get_tx (SCM);
SCM scheme_clef_directive_get_ty (SCM);
SCM scheme_clef_directive_get_gx (SCM);
SCM scheme_clef_directive_get_gy (SCM);
SCM scheme_clef_directive_get_override (SCM);
SCM scheme_clef_directive_get_width (SCM);
SCM scheme_clef_directive_get_height (SCM);
SCM scheme_timesig_directive_put_display (SCM tag, SCM value);
SCM scheme_timesig_directive_put_prefix (SCM tag, SCM value);
SCM scheme_timesig_directive_put_postfix (SCM tag, SCM value);
SCM scheme_timesig_directive_put_graphic (SCM tag, SCM value);
SCM scheme_timesig_directive_get_display (SCM);
SCM scheme_timesig_directive_get_prefix (SCM);
SCM scheme_timesig_directive_get_postfix (SCM);
SCM scheme_timesig_directive_put_x (SCM tag, SCM value);
SCM scheme_timesig_directive_put_y (SCM tag, SCM value);
SCM scheme_timesig_directive_put_tx (SCM tag, SCM value);
SCM scheme_timesig_directive_put_ty (SCM tag, SCM value);
SCM scheme_timesig_directive_put_gx (SCM tag, SCM value);
SCM scheme_timesig_directive_put_gy (SCM tag, SCM value);
SCM scheme_timesig_directive_put_override (SCM tag, SCM value);
SCM scheme_timesig_directive_get_x (SCM);
SCM scheme_timesig_directive_get_y (SCM);
SCM scheme_timesig_directive_get_tx (SCM);
SCM scheme_timesig_directive_get_ty (SCM);
SCM scheme_timesig_directive_get_gx (SCM);
SCM scheme_timesig_directive_get_gy (SCM);
SCM scheme_timesig_directive_get_override (SCM);
SCM scheme_timesig_directive_get_width (SCM);
SCM scheme_timesig_directive_get_height (SCM);
SCM scheme_tuplet_directive_put_display (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_prefix (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_postfix (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_graphic (SCM tag, SCM value);
SCM scheme_tuplet_directive_get_display (SCM);
SCM scheme_tuplet_directive_get_prefix (SCM);
SCM scheme_tuplet_directive_get_postfix (SCM);
SCM scheme_tuplet_directive_put_x (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_y (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_tx (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_ty (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_gx (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_gy (SCM tag, SCM value);
SCM scheme_tuplet_directive_put_override (SCM tag, SCM value);
SCM scheme_tuplet_directive_get_x (SCM);
SCM scheme_tuplet_directive_get_y (SCM);
SCM scheme_tuplet_directive_get_tx (SCM);
SCM scheme_tuplet_directive_get_ty (SCM);
SCM scheme_tuplet_directive_get_gx (SCM);
SCM scheme_tuplet_directive_get_gy (SCM);
SCM scheme_tuplet_directive_get_override (SCM);
SCM scheme_tuplet_directive_get_width (SCM);
SCM scheme_tuplet_directive_get_height (SCM);
SCM scheme_stemdirective_directive_put_display (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_prefix (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_postfix (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_graphic (SCM tag, SCM value);
SCM scheme_stemdirective_directive_get_display (SCM);
SCM scheme_stemdirective_directive_get_prefix (SCM);
SCM scheme_stemdirective_directive_get_postfix (SCM);
SCM scheme_stemdirective_directive_put_x (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_y (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_tx (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_ty (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_gx (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_gy (SCM tag, SCM value);
SCM scheme_stemdirective_directive_put_override (SCM tag, SCM value);
SCM scheme_stemdirective_directive_get_x (SCM);
SCM scheme_stemdirective_directive_get_y (SCM);
SCM scheme_stemdirective_directive_get_tx (SCM);
SCM scheme_stemdirective_directive_get_ty (SCM);
SCM scheme_stemdirective_directive_get_gx (SCM);
SCM scheme_stemdirective_directive_get_gy (SCM);
SCM scheme_stemdirective_directive_get_override (SCM);
SCM scheme_stemdirective_directive_get_width (SCM);
SCM scheme_stemdirective_directive_get_height (SCM);
SCM scheme_keysig_directive_put_display (SCM tag, SCM value);
SCM scheme_keysig_directive_put_prefix (SCM tag, SCM value);
SCM scheme_keysig_directive_put_postfix (SCM tag, SCM value);
SCM scheme_keysig_directive_put_graphic (SCM tag, SCM value);
SCM scheme_keysig_directive_get_display (SCM);
SCM scheme_keysig_directive_get_prefix (SCM);
SCM scheme_keysig_directive_get_postfix (SCM);
SCM scheme_keysig_directive_put_x (SCM tag, SCM value);
SCM scheme_keysig_directive_put_y (SCM tag, SCM value);
SCM scheme_keysig_directive_put_tx (SCM tag, SCM value);
SCM scheme_keysig_directive_put_ty (SCM tag, SCM value);
SCM scheme_keysig_directive_put_gx (SCM tag, SCM value);
SCM scheme_keysig_directive_put_gy (SCM tag, SCM value);
SCM scheme_keysig_directive_put_override (SCM tag, SCM value);
SCM scheme_keysig_directive_get_x (SCM);
SCM scheme_keysig_directive_get_y (SCM);
SCM scheme_keysig_directive_get_tx (SCM);
SCM scheme_keysig_directive_get_ty (SCM);
SCM scheme_keysig_directive_get_gx (SCM);
SCM scheme_keysig_directive_get_gy (SCM);
SCM scheme_keysig_directive_get_override (SCM);
SCM scheme_keysig_directive_get_width (SCM);
SCM scheme_keysig_directive_get_height (SCM);
SCM scheme_scoreheader_directive_put_display (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_prefix (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_postfix (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_graphic (SCM tag, SCM value);
SCM scheme_scoreheader_directive_get_display (SCM);
SCM scheme_scoreheader_directive_get_prefix (SCM);
SCM scheme_scoreheader_directive_get_postfix (SCM);
SCM scheme_scoreheader_directive_put_x (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_y (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_tx (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_ty (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_gx (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_gy (SCM tag, SCM value);
SCM scheme_scoreheader_directive_put_override (SCM tag, SCM value);
SCM scheme_scoreheader_directive_get_x (SCM);
SCM scheme_scoreheader_directive_get_y (SCM);
SCM scheme_scoreheader_directive_get_tx (SCM);
SCM scheme_scoreheader_directive_get_ty (SCM);
SCM scheme_scoreheader_directive_get_gx (SCM);
SCM scheme_scoreheader_directive_get_gy (SCM);
SCM scheme_scoreheader_directive_get_override (SCM);
SCM scheme_scoreheader_directive_get_width (SCM);
SCM scheme_scoreheader_directive_get_height (SCM);
SCM scheme_header_directive_put_display (SCM tag, SCM value);
SCM scheme_header_directive_put_prefix (SCM tag, SCM value);
SCM scheme_header_directive_put_postfix (SCM tag, SCM value);
SCM scheme_header_directive_put_graphic (SCM tag, SCM value);
SCM scheme_header_directive_get_display (SCM);
SCM scheme_header_directive_get_prefix (SCM);
SCM scheme_header_directive_get_postfix (SCM);
SCM scheme_header_directive_put_x (SCM tag, SCM value);
SCM scheme_header_directive_put_y (SCM tag, SCM value);
SCM scheme_header_directive_put_tx (SCM tag, SCM value);
SCM scheme_header_directive_put_ty (SCM tag, SCM value);
SCM scheme_header_directive_put_gx (SCM tag, SCM value);
SCM scheme_header_directive_put_gy (SCM tag, SCM value);
SCM scheme_header_directive_put_override (SCM tag, SCM value);
SCM scheme_header_directive_get_x (SCM);
SCM scheme_header_directive_get_y (SCM);
SCM scheme_header_directive_get_tx (SCM);
SCM scheme_header_directive_get_ty (SCM);
SCM scheme_header_directive_get_gx (SCM);
SCM scheme_header_directive_get_gy (SCM);
SCM scheme_header_directive_get_override (SCM);
SCM scheme_header_directive_get_width (SCM);
SCM scheme_header_directive_get_height (SCM);
SCM scheme_paper_directive_put_display (SCM tag, SCM value);
SCM scheme_paper_directive_put_prefix (SCM tag, SCM value);
SCM scheme_paper_directive_put_postfix (SCM tag, SCM value);
SCM scheme_paper_directive_put_graphic (SCM tag, SCM value);
SCM scheme_paper_directive_get_display (SCM);
SCM scheme_paper_directive_get_prefix (SCM);
SCM scheme_paper_directive_get_postfix (SCM);
SCM scheme_paper_directive_put_x (SCM tag, SCM value);
SCM scheme_paper_directive_put_y (SCM tag, SCM value);
SCM scheme_paper_directive_put_tx (SCM tag, SCM value);
SCM scheme_paper_directive_put_ty (SCM tag, SCM value);
SCM scheme_paper_directive_put_gx (SCM tag, SCM value);
SCM scheme_paper_directive_put_gy (SCM tag, SCM value);
SCM scheme_paper_directive_put_override (SCM tag, SCM value);
SCM scheme_paper_directive_get_x (SCM);
SCM scheme_paper_directive_get_y (SCM);
SCM scheme_paper_directive_get_tx (SCM);
SCM scheme_paper_directive_get_ty (SCM);
SCM scheme_paper_directive_get_gx (SCM);
SCM scheme_paper_directive_get_gy (SCM);
SCM scheme_paper_directive_get_override (SCM);
SCM scheme_paper_directive_get_width (SCM);
SCM scheme_paper_directive_get_height (SCM);
SCM scheme_layout_directive_put_display (SCM tag, SCM value);
SCM scheme_layout_directive_put_prefix (SCM tag, SCM value);
SCM scheme_layout_directive_put_postfix (SCM tag, SCM value);
SCM scheme_layout_directive_put_graphic (SCM tag, SCM value);
SCM scheme_layout_directive_get_display (SCM);
SCM scheme_layout_directive_get_prefix (SCM);
SCM scheme_layout_directive_get_postfix (SCM);
SCM scheme_layout_directive_put_x (SCM tag, SCM value);
SCM scheme_layout_directive_put_y (SCM tag, SCM value);
SCM scheme_layout_directive_put_tx (SCM tag, SCM value);
SCM scheme_layout_directive_put_ty (SCM tag, SCM value);
SCM scheme_layout_directive_put_gx (SCM tag, SCM value);
SCM scheme_layout_directive_put_gy (SCM tag, SCM value);
SCM scheme_layout_directive_put_override (SCM tag, SCM value);
SCM scheme_layout_directive_get_x (SCM);
SCM scheme_layout_directive_get_y (SCM);
SCM scheme_layout_directive_get_tx (SCM);
SCM scheme_layout_directive_get_ty (SCM);
SCM scheme_layout_directive_get_gx (SCM);
SCM scheme_layout_directive_get_gy (SCM);
SCM scheme_layout_directive_get_override (SCM);
SCM scheme_layout_directive_get_width (SCM);
SCM scheme_layout_directive_get_height (SCM);
SCM scheme_movementcontrol_directive_put_display (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_prefix (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_postfix (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_graphic (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_get_display (SCM);
SCM scheme_movementcontrol_directive_get_prefix (SCM);
SCM scheme_movementcontrol_directive_get_postfix (SCM);
SCM scheme_movementcontrol_directive_put_x (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_y (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_tx (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_ty (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_gx (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_gy (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_put_override (SCM tag, SCM value);
SCM scheme_movementcontrol_directive_get_x (SCM);
SCM scheme_movementcontrol_directive_get_y (SCM);
SCM scheme_movementcontrol_directive_get_tx (SCM);
SCM scheme_movementcontrol_directive_get_ty (SCM);
SCM scheme_movementcontrol_directive_get_gx (SCM);
SCM scheme_movementcontrol_directive_get_gy (SCM);
SCM scheme_movementcontrol_directive_get_override (SCM);
SCM scheme_movementcontrol_directive_get_width (SCM);
SCM scheme_movementcontrol_directive_get_height (SCM);
SCM scheme_log_debug(SCM message);
SCM scheme_log_info(SCM message);
SCM scheme_log_message(SCM message);
SCM scheme_log_warning(SCM message);
SCM scheme_log_critical(SCM message);
SCM scheme_log_error(SCM message);
#endif
