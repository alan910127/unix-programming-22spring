    global write:function
    global alarm:function
    global sys_pause:function
    global exit:function
    global sleep:function
    global setjmp:function
    global longjmp:function
    global __myrt:function
    global sys_rt_sigaction:function
    global sys_rt_sigpending:function
    global sys_rt_sigprocmask:function

write:  ; rdi = fd, rsi = buffer, rdx = length
    mov rax, 1
    syscall
    ret


alarm:  ; rdi = seconds
    mov rax, 37
    syscall
    ret


sys_pause:  ; no arguments
    mov rax, 34
    syscall
    ret


exit:   ; rdi = status
    mov rax, 60
    syscall
    ret


sleep:  ; rdi = seconds
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


setjmp: ; rdi = buf
    pop rcx
    mov QWORD [rdi + 0x00], rcx
    mov QWORD [rdi + 0x08], r15
    mov QWORD [rdi + 0x10], r14
    mov QWORD [rdi + 0x18], r13
    mov QWORD [rdi + 0x20], r12
    mov QWORD [rdi + 0x28], rbp
    mov QWORD [rdi + 0x30], rsp
    mov QWORD [rdi + 0x38], rbx

    ; preserve signal mask
    push rdi
    push rsi
    push rdx
    push rcx

    mov rdi, 0
    mov rsi, 0  ; set = NULL
    lea rdx, [rdi + 0x40]
    mov r10, 8
    call sys_rt_sigprocmask

    pop rcx
    pop rdx
    pop rsi
    pop rdi

    xor rax, rax
    jmp rcx

longjmp: ; rdi = buf, rsi = ret
    mov rcx, QWORD [rdi + 0x00] 
    mov r15, QWORD [rdi + 0x08] 
    mov r14, QWORD [rdi + 0x10] 
    mov r13, QWORD [rdi + 0x18] 
    mov r12, QWORD [rdi + 0x20] 
    mov rbp, QWORD [rdi + 0x28] 
    mov rsp, QWORD [rdi + 0x30] 
    mov rbx, QWORD [rdi + 0x38]

    ; restore signal mask
    push rdi
    push rsi
    push rdx
    push rcx

    mov rdi, 2  ; SIG_SETMASK
    lea rsi, [rdi + 0x40]
    mov rdx, 0  ; oldset = NULL
    mov r10, 8
    call sys_rt_sigprocmask

    pop rcx
    pop rdx
    pop rsi
    pop rdi

    mov rax, rsi
    test rax, rax
    jnz longjmp_finish
    inc rax
longjmp_finish:
    jmp rcx


__myrt:
    mov rax, 15
    syscall
    ret

sys_rt_sigaction:   ; rdi = signum, rsi = act, rdx = oldact, rcx = sigsetsize
    push r10
    mov r10, rcx
    mov rax, 13         ; sys_rt_sigaction
    syscall  
    pop r10
    ret

sys_rt_sigprocmask: ; rdi = how, rsi = set, rdx = oldset, rcx = sigsetsize
    push r10
    mov r10, rcx
    mov rax, 14         ; sys_rt_sigprocmask
    syscall
    pop r10
    ret

sys_rt_sigpending: ; rdi = set, rsi = sigsetsize
    mov rax, 127        ; sys_rt_sigpending
    syscall
    ret