
.global NOP3x1AllBytes
.global NOP1x3AllBytes
.global NOP1x9AllBytes

.text

NOP3x1AllBytes:
    xor %rax, %rax
.loopNOP3x1:
    nopl (%rax)
    inc %rax
    cmp %rdi, %rax
    jb .loopNOP3x1
    ret

NOP1x3AllBytes:
    xor %rax, %rax
.loopNOP1x3:
    nop
    nop
    nop
    inc %rax
    cmp %rdi, %rax
    jb .loopNOP1x3
    ret

NOP1x9AllBytes:
    xor %rax, %rax
.loopNOP1x9:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    inc %rax
    cmp %rdi, %rax
    jb .loopNOP1x9
    ret
