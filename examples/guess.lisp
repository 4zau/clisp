(print "loading random module...")
(load-plugin "./plugins/random.so")

(if (exists goal) (return_nil) (def goal (% (get_random) 100)))
(print "goal exists")

; this was written before I made io.c
; comments were implemented just to write this
(print "checking your 'answer'")
(if (exists answer)
    (if (= answer goal) 
        (print "correct!")
        (if (> answer goal) 
            (print "answer too high")
            (print "answer too low")
        )
    )
    (print "please define an 'answer'")
)