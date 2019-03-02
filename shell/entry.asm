global  _start
extern main,exit
        [section .text]
_start:
        call    main
        jmp     $
        push    eax
        call    exit
        jmp     $
        add     esp,4
        jmp     $
