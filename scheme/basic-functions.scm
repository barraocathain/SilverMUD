(define (create-two-way-path from-path-description to-path-description from-area to-area arealist)
							 (begin
							   (create-path from-path-description from-area to-area arealist)
							   (create-path to-path-description to-area from-area arealist)))
