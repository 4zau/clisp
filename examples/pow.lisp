(print "loading pow module...")
(load-plugin "../plugins/pow.so")

(print "--- test power function ---")
(def a (pow 2 10))
(print "two to the power of ten:")
(print a)

(print "--- three to the power of four ---")
(def b (pow 3 4))
(print b)

(print "--- lambda test ---")
(def square (lambda (x) (pow x 2)))
(print "square of 20:")
(print (square 20))

(print "--- error handling ---")
(print (pow 5 -2))