(load-plugin "./plugins/random.so")
(load-plugin "./plugins/io.so")

(def num-to-move (lambda (n)
  (if (= n 0) "rock"
  (if (= n 1) "paper"
              "scissors"))))

(def play-round (lambda! ()
  (begin
    (print "------------------------------")
    (print "type 'rock', 'paper', or 'scissors':")
    (def user (read-string))
    
    (def comp (num-to-move (% (get_random) 3)))
    (print "computer played:")
    (print comp)
    
    (if (= user comp)
        (print "result: it's a tie!")
        (if (= user "rock")
            (if (= comp "scissors") (print "result: you win!") (print "result: you lose!"))
        (if (= user "paper")
            (if (= comp "rock") (print "result: you win!") (print "result: you lose!"))
        (if (= user "scissors")
            (if (= comp "paper") (print "result: you win!") (print "result: you lose!"))
            (print "invalid input, please try again")))))
    
    (print "")
    (play-round))))

(print "rock paper scissors!")
(play-round)