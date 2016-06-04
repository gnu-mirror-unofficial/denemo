;;;; NewStructuredStaff
(let ((params NewStructuredStaff::params))
    (d-MoveToBeginning)
    (if (eq? params 'initial)
        (d-AddInitial)
        (d-AddAfter))
    (if (eq? params 'voice)
        (d-SetCurrentStaffAsVoice))
    (if (not (eq? params 'initial))
        (d-MoveToStaffUp)
        (d-MoveToStaffDown))
        
    (if (or (Timesignature?) (d-Directive-standalone? "Upbeat")(d-Directive-standalone? "ShortMeasure"))
            (begin
                (d-PushPosition)
                (d-UnsetMark)
                (d-SetMark)
                (d-Copy)
                (d-MoveToStaffDown)
                (d-Paste) 
                (d-PopPosition)))
    (while (d-MoveCursorRight)
        (if (or (Timesignature?) (d-Directive-standalone? "Upbeat")(d-Directive-standalone? "ShortMeasure"))
            (begin
                (d-PushPosition)
                (d-UnsetMark)
                (d-SetMark)
                (d-Copy)
                (if (eq? params 'initial)
                    (d-MoveToStaffUp)
                    (d-MoveToStaffDown))

                (d-Paste) 
                (d-PopPosition))))
    (d-MoveToStaffDown)
    (d-MoveToBeginning))
