;;;ReplicateManualBeams
;creates a template beaming pattern from the selection (a list of pairs comprising a duration and a type ("[" "]" "n" for no beam, + - n for n beams right/left)
;then searches onwards in staff for the same pattern on the same beat and applies it where found in the same time signature.
(let ((timesig (d-GetPrevailingTimesig))(start-tick 0)(current-tick 0)(pattern-ticks 0)(beam "Beam")(beamright "BeamRight")(beamleft "BeamLeft")(nobeam "NoBeam"))
	(define (selected-ticks)
		(define ticks 0)
		(if (d-GoToSelectionStart)
			(let ((start (d-GetMeasure)))
				(while (d-IsInSelection)
					(set! ticks (+ ticks (d-GetDurationInTicks)))
					(d-MoveCursorRight))
				;(disp "Total ticks " ticks "in measure " start " ending at " (d-GetMeasure) "\n")
				(if (= (d-GetMeasure) start)
					ticks
					#f))
			#f))
	(define (get-beam-info)
		(if (d-Directive-chord? beam)
			(d-DirectiveGet-chord-display beam)
			(if (d-DirectiveGet-chord-display beamright)
				(string->number (string-copy (d-DirectiveGet-chord-display beamright) 1 2))
				(if (d-DirectiveGet-chord-display beamleft)
					(- 0 (string->number (string-copy (d-DirectiveGet-chord-display beamleft) 1 2)))
					(if (d-Directive-chord? nobeam)
						"n"
						(if (Rest?)
							"r"
							#f))))))				
	(define (get-note-info)
			(define tick (d-GetDurationInTicks))
			(set! pattern-ticks (+ tick pattern-ticks))
			(cons tick (get-beam-info)))
	(define (get-beams)
		(define beams '())
		(let loop ()
			(if (Music?)
				(set! beams (cons (get-note-info) beams)))
			(if (and (d-NextChordInMeasure) (d-IsInSelection))
				(loop)))
		(reverse beams))
	(define (match-rhythm beams)
		(define found #f)
		(if (not (Music?))
			(d-NextChordInMeasure))
		(if (Music?)
			(let loop ()
				(define tick (d-GetDurationInTicks))
					;(disp "Trying for a match at " (GetPosition) " with note ticks = " tick " and current-tick position " current-tick " for to match pattern start at " start-tick " pattern length - " pattern-ticks " ticks\n\n")
					(if (and (zero? tick) (d-NextChordInMeasure))
						(loop);;here skip over anything of 0 duration
						(if (not (= current-tick start-tick)) ;skip if we are not on the same beat
							(begin ;(disp "Not on the beat at " (GetPosition) " current tick " current-tick "\n")
								(set! current-tick (+ current-tick tick))
								(if (d-NextChordInMeasure)
									(loop)))
							(let ((position (GetPosition)))
								(let inner-loop ((remaining-list beams)) 
									;(disp "Inner loop remaining list " remaining-list "\n\n")
									(set! tick (d-GetDurationInTicks))
									(if (null? remaining-list)
										(set! found position) ;we are finished with a match
										(if (and (equal? (cdar remaining-list) "r") (not (Rest?)))
											(begin ;a rest was required but there is none
												(set! current-tick (+ current-tick tick))
												(if (d-NextChordInMeasure)
													(loop)))
											(begin					
												(set! current-tick (+ current-tick tick))
												(if (= tick (caar remaining-list))
													(if (or (null? (cdr remaining-list)) (d-NextChordInMeasure))
														(inner-loop (cdr remaining-list)))))))))))))
		(if (and (not found) (d-NextChordInMeasure))
			(match-rhythm beams)
			found))	
	(define (set-beam type) ;(disp "set-beam called with " type "\n\n")
		;clear current beam directives
		(d-DirectiveDelete-chord nobeam)
		(d-DirectiveDelete-chord beam)
		(d-DirectiveDelete-chord beamright)
		(d-DirectiveDelete-chord beamleft)
		;install new ones
		(if type
			(begin
				(if (number? type)
					(begin
						(if (positive? type) 
								(BeamCount "Right" type)	
								(BeamCount "Left" (- 0 type))))
					(begin
						(if (d-Directive-chord? beam)
							(d-DirectiveDelete-chord beam))
						(if (equal? type "[")
							(d-StartBeam)
							(if (equal? type "]")
								(d-EndBeam)
								(if (equal? type "n")
									(d-NoBeam)	
								;; do nothing for rest
								))))))))
						
	(define (apply-rhythm beams)
		(let loop ((thelist beams))
			(if (not (null? thelist))
				(begin
					(if (and (zero? (d-GetDurationInTicks)) (d-MoveCursorRight))
						(loop thelist)
						(begin
							;(disp "setting beam at " (GetPosition) " tick " (d-GetDurationInTicks) " for " (caar thelist) " . " (cdar thelist) "\n")
							(set-beam (cdar thelist))
							(if (d-MoveCursorRight)
								(loop (cdr thelist)))))))))
				
	;;actual procedure.
	(d-PushPosition)
	(d-GoToSelectionStart)
	(GoToMeasureBeginning)
	(let loop ()
		(if (not (d-IsInSelection))
			(set! start-tick (+ start-tick (d-GetDurationInTicks))))
		(if (and (not (d-IsInSelection)) (d-NextChordInMeasure))
				(loop)))

	(if (selected-ticks)
		(let ((beams '()))
			(d-GoToSelectionStart)
			(set! beams (get-beams))
			;(disp "beams are " beams "start-tick " start-tick "\n")
			(GoToMeasureBeginning)
			(set! current-tick 0)
			(let loop ()
				(define position  (match-rhythm beams))
				(set! current-tick 0)
				;(disp "At measure " (d-GetMeasure) " found pattern at " position "\n")
				(if position
					(begin
						(apply d-GoToPosition position)
						(apply-rhythm beams)))
				(let skip-wrong-timesig ()
					(if (d-MoveToMeasureRight)
						(if (equal? (d-GetPrevailingTimesig) timesig)
							(loop)
							(skip-wrong-timesig))))))
		(begin
			(d-WarningDialog (_ "Selection must start and end in the same bar" ))))
		(d-PopPosition))
