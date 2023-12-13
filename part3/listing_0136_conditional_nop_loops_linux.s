
.global ConditionalNOP

.text

ConditionalNOP:
    xor %rax, %rax
.loop:
    mov (%rsi, %rax, 1), %r10
    inc %rax
    test $1, %r10
    jnz .skip
    nop
.skip:
    cmp %rdi, %rax
    jb .loop
    ret
