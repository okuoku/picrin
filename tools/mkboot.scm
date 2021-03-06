(import (scheme base)
        (scheme read)
        (scheme write))

(define (with-output-to-string thunk)
  (let ((port (open-output-string)))
    (parameterize ((current-output-port port))
      (thunk)
      (let ((s (get-output-string port)))
        (close-port port)
        s))))

(define exprs
  (let loop ((acc '()))
    (let ((e (read)))
      (if (eof-object? e)
          (reverse acc)
          (loop (cons e acc))))))

(define text
  (with-output-to-string
    (lambda ()
      (for-each
       (lambda (e)
         (write e)
         (write-string " "))
       exprs))))

(define (escape-string s)
  (with-output-to-string
    (lambda ()
      (string-for-each
       (lambda (c)
         (case c
           ((#\\) (write-string "\\\\"))
           ((#\") (write-string "\\\""))
           ((#\newline) (write-string "\\n"))
           (else (write-char c))))
       s))))

(define (group-string i s)
  (let loop ((t s) (n (string-length s)) (acc '()))
    (if (= n 0)
        (reverse acc)
        (if (< n i)
            (loop "" 0 (cons t acc))
            (loop (string-copy t i) (- n i) (cons (string-copy t 0 i) acc))))))

(define lines (map escape-string (group-string 80 text)))

(for-each
 (lambda (s) (display s) (newline))
 `("#include \"picrin.h\""
   "#include \"picrin/extra.h\""
   ""
   "static const char boot_rom[][80] = {"
   ,@(let loop ((lines lines) (acc '()))
       (if (null? lines)
           (reverse acc)
           (loop (cdr lines) (cons (string-append "\"" (car lines) "\",") acc))))
   "};"
   ""
   "void"
   "pic_boot(pic_state *pic)"
   "{"
   "  pic_load_cstr(pic, &boot_rom[0][0]);"
   "}"))

