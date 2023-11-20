;;;; This file is part of SilverMUD.
;;;; structures.scm defines various variables and functions used for interacting with C structures
;;;; from the Scheme enviroment of SilverMUD.
(define-module (silvermud structures))
(use-modules (system foreign)
			 (silvermud primitives))

;;; struct PlayerList:

;; The layout of the struct PlayerList:
(define *player-list-structure* (list size_t '* '*))

;; Pretty-format the player list header:
(define (player-list-header->string player-list-pointer)
  "Format a struct PlayerList pointer into a string."
  (if (not (null-pointer? player-list-pointer))
	  (let ((structure (parse-c-struct player-list-pointer *player-list-structure*)))
		(format #f
				"Players in list: ~d.\nHead: ~a. \nTail: ~a.\n"
				(list-ref structure 0) (list-ref structure 1) (list-ref structure 2)))))

;; Create a list of strings representing all players in a list:
(define (list-players player-list-pointer)
  "List all players in a given C PlayerList as a list of strings."
  (if (not (null-pointer? player-list-pointer)) ; Check we're not dereferencing a null pointer.
	  (build-list-players
	   (list-ref (parse-c-struct player-list-pointer *player-list-node-structure*) 2))))

(define (build-list-players pointer)
  (if (not (null-pointer? pointer))
	  (let* ((node (parse-c-struct pointer *player-list-node-structure*))
			(player (parse-c-struct (list-ref node 0) *player-structure*)))
		(cons (pointer->string (list-ref player 1)) (build-list-players (list-ref node 1))))
	  '()))

;;; struct PlayerListNode:

;; Used to interact with struct PlayerListNode:
(define *player-list-node-structure* (list '* '* '*))

;; Pretty-format the player list node:
(define (player-list-node->string player-list-node-pointer)
  "Format a struct PlayerListNode pointer into a string."
  (if (not (null-pointer? player-list-node-pointer))
	  (let ((structure (parse-c-struct player-list-node-pointer *player-list-node-structure*)))
		(format #f
				"Player pointer: ~a.\nNext: ~a. \nPrevious: ~a.\n"
				(list-ref structure 0) (list-ref structure 1) (list-ref structure 2)))))

;;; struct Player:

; Used to interact with struct Player:
(define *player-structure* (list '* '*))

(define (player->string player-info-pointer)
  "Format a struct Player pointer into a string."
  (if (not (null-pointer? player-info-pointer))
	  (let ((structure (parse-c-struct player-info-pointer *player-structure*)))
		(display (null-pointer? (list-ref structure 1)))
		(format #f
				"Player Name: ~a\n" (if (null-pointer? (list-ref structure 1))	 
										(pointer->bytevector (list-ref structure 1) 64)
										#f)))))

;; Export everything!
(export *player-list-structure* *player-list-node-structure* *player-structure*
		player->string player-list-header->string player-list-node->string list-players)
