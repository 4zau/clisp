; more of a set! showcase tbh

(def energy 10)
(def hunger 5)

(def play-with-pet (lambda! ()
  (if (< energy 2)
      (print "pet is too tired to play! need sleep.")
      (begin
        (set! energy (- energy 2))
        (set! hunger (+ hunger 3))
        (print "you played with the pet!")))))

(def feed-pet (lambda! ()
  (begin
    (set! hunger 0)
    (set! energy (+ energy 1))
    (print "nom nom nom..."))))

(def sleep (lambda! ()
  (begin
    (set! energy 10)
    (set! hunger (+ hunger 1))
    (print "zzzz..."))))

(def status (lambda! ()
  (begin
    (print "energy:") (print energy)
    (print "hunger:") (print hunger))))