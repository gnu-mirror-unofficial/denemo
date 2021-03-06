; d-Performance
(define Performance::timings '())
(define (Performance::play params)
	 (if (not (null? Performance::timings))
	  (begin
		(d-SetPlaybackInterval (car (car (car Performance::timings))) (cdr (car Performance::timings)))
		(apply d-GoToPosition (cdr (car (car Performance::timings))))
		(set! Performance::timings (list-tail Performance::timings 1))
		(d-Play (string-append "(Performance::play \"" (if params (scheme-escape params) "#f") "\")")))
	  (begin ;finished playing all repeats
			(if params (eval-string params)))))
			
(let ((params Performance::params))
	(d-CreateTimebase)
	(d-MoveToBeginning)
	(while (and (not (Music?)) (d-NextObject))) 	
	(if (d-AudioIsPlaying)
		(begin
			(d-Stop)
			(d-OneShotTimer 10 "(d-Stop)")
			(d-OneShotTimer 15 "(d-Stop)")
			(d-OneShotTimer 20 "(d-Stop)"))
		(let ((beginning (GetPosition))
			 (start-time (d-GetMidiOnTime))
			 (position #f)(s1 #f)(e1 #f)(s2 #f)(e2 #f)(fine #f)(segno #f)(segno-position #f)(timing (d-GetMidiOffTime))(first-time #f))
		  (set! s1 (cons start-time  beginning))
		  (while (d-NextObject)
			(if (and (d-GetMidiOffTime) (>  (d-GetMidiOffTime)  0))
			  (set! timing (d-GetMidiOffTime)))
			;(disp "MIDI off time " timing " ok")
			(set! position (GetPosition))
			(set! first-time (d-DirectiveGet-standalone-data "OpenNthTimeBar"))
			(if first-time
				(begin
					(set! first-time (eval-string first-time))
					(set! first-time (assq-ref first-time 'volta))))
			;;;(disp "first time is " first-time "\n\n")
			(cond
			  ((Music?)
				  (if (d-Directive-chord? "DCAlFine")
					(begin
					  ;(disp "DC al fine performing " s1 " to " timing "first")
					  (set! Performance::timings (cons (cons s1 timing) Performance::timings))
					  (set! s1 (cons start-time beginning))   
					  (set! e1 timing)
					  (if fine
						(set! e1 fine)
						(d-InfoDialog (_ "No fine found, assuming all")))))
				   (if (d-Directive-chord? "DalSegno")
					(begin
					  (set! Performance::timings (cons (cons s1 timing) Performance::timings))
					  (set! s1 (cons start-time beginning)) ;; in case no segno found
					  (if segno
						(set! s1 (cons segno segno-position))
						(d-InfoDialog (_ "Dal Segno with no Segno - assuming Da Capo")))
					  (set! e1 timing)
					  (if fine
						(set! e1 fine)
						(d-InfoDialog (_ "No fine found, assuming all")))))
				  (if (d-Directive-chord? "ToggleFine")
					  (set! fine timing))
				   (if (d-Directive-chord? "ToggleSegno")
					  (begin
						(set! segno-position (GetPosition))
						(set! segno (d-GetMidiOnTime))))
				  )
			  ((or (d-Directive-standalone? "RepeatEnd") (d-Directive-standalone? "RepeatEndStart"))
				  (set! e1 timing)
				  ;(disp "Midi repeat from " e1 " ok")
				  (set! Performance::timings (cons (cons s1 e1) Performance::timings))
				  ;(disp "timings at repeat end " Performance::timings " ok")
				  (if (not e2)
						(set! e2 e1))
				  (if (not s2)
						(set! s2 s1))
					
				  (set! Performance::timings (cons (cons s2 e2) Performance::timings))
				  ;(disp "timings at repeat " Performance::timings " ok")
				  (set! s1 (cons timing (GetPosition)))
				  (set! e1 #f)
				  (set! s2 #f)
				  (set! e2 #f)
				  (if (d-Directive-standalone? "RepeatEndStart")
					(set! s2 (cons timing (GetPosition))))
				  )
			  ((d-Directive-standalone? "RepeatStart")
				  (set! s2 (cons timing (GetPosition))))
			  ((and first-time (= first-time 1))
				(set! e2 timing)
				;(disp "Midi 2 repeat to " e2 " ok")
				(if (not s2)
				  (set! s2 (cons start-time beginning)))
				  )
			)
		  )
		(if (not e1)
		  (set! e1 timing))
		(if (not (equal? (car s1) timing))
			(set! Performance::timings (cons (cons s1 e1) Performance::timings)))
		(set! Performance::timings (reverse Performance::timings))
		(disp "Playing with " Performance::timings " and params " params "\n\n")
		(Performance::play params))))

