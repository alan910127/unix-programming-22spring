write:
    mov rax, 1
    syscall
    ret

alarm:
    mov rax, 37
    syscall
    ret

pause:
    mov rax, 34
    syscall
    ret

exit:
    mov rax, 60
    syscall
    ret

sleep:
    sub rsp, 32             ; sizeof(struct timespec) * 2
    mov [rsp], rdi          ; req.tv_sec = seconds
    mov QWORD [rsp+8], 0    ; req.tv_nsec = 0
    mov rdi, rsp            ; rdi = &req
    lea rsi, [rsp+16]       ; rsi = &rem
    mov rax, 35             ; nanosleep
    syscall
    cmp rax, 0
    jge sleep_quit          ; no error
sleep_error:
    neg rax
    cmp rax, 4              ; EINTR = 4
    jne sleep_failed
sleep_interrupted:
    mov rax, [rsp+16]       ; return value = rem.tv_sec
    jmp sleep_quit
sleep_failed:
    mov rax, 0
sleep_quit:
    add rsp, 32             ; revert the stack
    ret
