
.global MOVAllBytesASM
.global NOPAllBytesASM
.global CMPAllBytesASM
.global DECAllBytesASM

.text

MOVAllBytesASM:
    xor %rax, %rax
.loopMOV:
    mov %al, (%rsi,%rax,1)
    inc %rax
    cmp %rdi, %rax
    jb .loopMOV
    ret

NOPAllBytesASM:
    xor %rax, %rax
.loopNOP:
    nopl (%rax)
    inc %rax
    cmp %rdi, %rax
    jb .loopNOP
    ret

CMPAllBytesASM:
    xor %rax, %rax
.loopCMP:
    inc %rax
    cmp %rdi, %rax
    jb .loopCMP
    ret

DECAllBytesASM:
.loopDEC:
    dec %rdi
    jnz .loopDEC
    ret
