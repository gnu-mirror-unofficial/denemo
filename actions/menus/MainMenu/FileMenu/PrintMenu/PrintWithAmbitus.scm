;;;;;;;;;PrintWithAmbitus
(let ((tag "Ambitus"))
    (define (toggleAmbitus)
      (while (d-PreviousMovement))
      (let movement ()
	(while (d-MoveToStaffUp))
	(let staff ()
	    (if (d-Directive-staff? tag)
		(d-DirectiveDelete-staff tag)
		(begin
		     (d-DirectivePut-staff-prefix tag "\\consists \"Ambitus_engraver\"\n")
		     (d-DirectivePut-staff-override tag  (logior DENEMO_ALT_OVERRIDE  DENEMO_OVERRIDE_AFFIX  DENEMO_OVERRIDE_GRAPHIC))))
	     (if (or (d-MoveToVoiceDown) (d-MoveToStaffDown))
		    (staff)))
		    (if (d-NextMovement)
			(movement))))

    (d-PushPosition)
    (if (d-ContinuousTypesetting)
   	 (d-WarningDialog (_ "Turn off continuous typesetting first"))
	    (let ((name (_ "Ambitus")))
		(d-DeleteLayout name)  
		(d-SelectDefaultLayout) 
		(toggleAmbitus)
		(d-SetPendingLayout name)
		(d-RefreshLilyPond)
		(d-CreateLayout name)  
		(d-SetPendingLayout #f)
		(d-SelectDefaultLayout)
		(toggleAmbitus)
		(d-WarningDialog (string-append (_ "Use Typeset->") name (_ " in the Print View to typeset your new layout")))
		(d-SetSaved #f)))
	
    (d-PopPosition))
