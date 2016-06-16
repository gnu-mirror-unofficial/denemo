;;;; NewStructuredStaff
(let ((params NewStructuredStaff::params))
    (define (copy-this)
                (d-PushPosition)
                (d-UnsetMark)
                (d-SetMark)
                (d-Copy)
                (if (not params)
                    (d-MoveToStaffDown)
                    (d-MoveToStaffUp))
                (d-Paste) 
                (d-PopPosition))
    (d-MoveToBeginning)
    (case params 
        ((initial)
        (d-AddInitial))
        ((before)
            (d-AddBefore))
        (else
            (d-AddAfter)))
    (if (eq? params 'voice)
        (begin
            (set! params #f)
            (d-SetCurrentStaffAsVoice)))
    (if (not params)
            (d-MoveToStaffUp) 
            (d-MoveToStaffDown))
    (if (or (Timesignature?) (d-Directive-standalone? "Upbeat") (d-Directive-standalone? "Blank") (d-Directive-standalone? "ShortMeasure"))
        (copy-this))
    (while (d-MoveCursorRight)
        (if (or (Timesignature?) (d-Directive-standalone? "Upbeat") (d-Directive-standalone? "Blank") (d-Directive-standalone? "ShortMeasure"))
            (copy-this)))
    (if (not params)
            (d-MoveToStaffDown) 
            (d-MoveToStaffUp))
     (d-SetSaved #f)
    (d-MoveToBeginning))
