;;;; This file is part of SilverMUD.
;;;; structures.scm defines various variables and functions used for interacting with the player's
;;;; via chat output from the Scheme enviroment of SilverMUD.
(define-module (silvermud messaging))
(use-modules (silvermud primitives))

(define message-everyone (lambda (name content)
						   (push-output-message *global-output-queue* #f *global-player-list*
												8 name content)))
(define system-message (lambda (contennt)
						 (push-output-message *global-output-queue* #f *global-player-list*
											  0 "" content)))

(define message-expression (lambda (expression)
							 (system-message (format #f "~a" expression))))

;; Export everything!
(export message-everyone system-message message-expression)
