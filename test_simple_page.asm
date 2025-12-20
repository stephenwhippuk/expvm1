DATA
PAGE data_page
message: DB "Hello"

CODE
START:
    LDA AX, message  ; Should inject PAGE instruction before this
    HALT
