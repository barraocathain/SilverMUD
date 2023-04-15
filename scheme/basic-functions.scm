(define (create-two-way-path from-path-description to-path-description from-area to-area arealist)
							 (begin
							   (create-path from-path-description from-area to-area arealist)
							   (create-path to-path-description to-area from-area arealist)))

;; Send a quick message to everyone in the game as the server:
(define (shout message)
  "Send a quick message to everyone in the game as the server."
  (message-everyone "SERVER" message output-queue))
