(load-plugin "./plugins/io.so")
(load-plugin "./plugins/raylib.so")
(load-plugin "./plugins/random.so")

(def NIL ())
(def screen-w 800)
(def screen-h 600)

(def state 0) ; 0 = playing, 1 = game over
(def score 0)
(def lives 3)

; player
(def px 350) (def py 520) (def pw 100) (def ph 20) (def p-speed 12)

; falling cubes
(def cx 400) (def cy 0) (def cw 30) (def c-speed 4)
(def cr 250) (def cg 50) (def cb 100)

(rl-init screen-w screen-h "peak")


(def check-collision (lambda! ()
    (if (< cx (+ px pw)) (if (> (+ cx cw) px)
    (if (< cy (+ py ph)) (if (> (+ cy cw) py) 1 NIL) NIL) NIL) NIL)
))

(def reset-cube (lambda! ()
    (begin
        (set! cy 0)
        (set! cx (% (get_random) (- screen-w cw)))
        (set! cr (+ 100 (% (get_random) 155)))
        (set! cg (+ 100 (% (get_random) 155)))
        (set! cb (+ 100 (% (get_random) 155)))
    )
))

; since im too lazy to add raylib's text drawing, 
; we resort to drawing lives and score with cubes
(def draw-lives (lambda! (l x)
    (if (> l 0)
        (begin
            (rl-draw-rect x 10 20 20  255 50 50 255)
            (draw-lives (- l 1) (+ x 30))
        )
        NIL)
))

; same
(def draw-score (lambda! (s x y)
    (if (> s 0)
        (begin
            (rl-draw-rect x y 10 20  250 200 50 255)
            (if (> x (- screen-w 30))
                ; wrap around
                (draw-score (- s 1) 10 (+ y 30))
                (draw-score (- s 1) (+ x 15) y)
            )
        )
        NIL)
))

; its cooler anyway

(def update-playing (lambda! ()
    (begin
        (if (rl-is-key-down 262) (if (< px (- screen-w pw)) (set! px (+ px p-speed)) NIL) NIL)
        (if (rl-is-key-down 263) (if (> px 0) (set! px (- px p-speed)) NIL) NIL)

        (set! cy (+ cy c-speed))

        (if (check-collision)
            (begin
                (reset-cube)
                (set! c-speed (+ c-speed 1))
                (set! score (+ score 1))
                (print "current score:" score)
            )
            NIL)

        (if (> cy screen-h)
            (begin
                (reset-cube)
                (set! c-speed 4)
                (set! lives (- lives 1))
                (print "uh oh, lives left:" lives)
                
                (if (< lives 1) 
                    (begin
                        (set! state 1)
                        (print "--- its over ---")
                        (print "final score:" score)
                        (print "press up to restart")
                    ) 
                    NIL)
            )
            NIL)

        (rl-clear 30 30 40 255)
        (rl-draw-rect px py pw ph  50 200 250 255)
        (rl-draw-rect cx cy cw cw  cr cg cb 255)
        (draw-lives lives 10)
    )
))

(def update-game-over (lambda! ()
    (begin
        (if (rl-is-key-down 265)
            (begin
                (set! state 0)
                (set! score 0)
                (set! lives 3)
                (set! c-speed 4)
                (reset-cube)
                (print "restarted!")
            )
            NIL)

        (rl-clear 100 30 30 255)
        
        (draw-score score 20 100)
    )
))


(def update-frame (lambda! ()
    (begin
        (rl-begin)
        
        (if (= state 0)
            (update-playing)
            (update-game-over)
        )

        (rl-end)
    )
))

(rl-game-loop update-frame)
(rl-close)