(load-plugin "./plugins/io.so")
(load-plugin "./plugins/raylib.so")

(rl-init 800 600 "raylib in lisp")

(def player-x 400)
(def player-y 300)
(def speed 5)

(def update-frame (lambda! ()
  (begin
    ; arrow keys in raylib: right=262, left=263, down=264, up=265
    (if (rl-is-key-down 262) (set! player-x (+ player-x speed)) ())
    (if (rl-is-key-down 263) (set! player-x (- player-x speed)) ())
    (if (rl-is-key-down 264) (set! player-y (+ player-y speed)) ())
    (if (rl-is-key-down 265) (set! player-y (- player-y speed)) ())

    (rl-begin)
    
    (rl-clear 30 30 40 255)
    
    (rl-draw-rect player-x player-y 50 50  250 80 100 255)
    
    (rl-end)))
)

(rl-game-loop update-frame)

(rl-close)
(print "raylib done running!")