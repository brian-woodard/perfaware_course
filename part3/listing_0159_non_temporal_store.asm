;  ========================================================================
;
;  (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 157
;  ========================================================================

global Test_Cache
global Test_Cache_Nt

section .text

;
; NOTE(casey): This ASM routine is written for the Windows 64-bit ABI.
;
;    rcx: block count
;    rdx: read data pointer
;     r8: write data pointer
;     r9: 
;

Test_Cache:
    xor rax, rax
    align 64
.read_loop:
    vmovdqu ymm0, [rdx]
    vmovdqu ymm0, [rdx + 32]
    vmovdqu ymm0, [rdx + 64]
    vmovdqu ymm0, [rdx + 96]

    add rax, 128
    cmp rax, rcx
    jb .read_loop

    mov rax, rdx
.write_loop:
    vmovdqu [rax], ymm0
    vmovdqu [rax + 32], ymm0
    vmovdqu [rax + 64], ymm0
    vmovdqu [rax + 96], ymm0
    add rax, 128
    sub rcx, 128
    jnz .write_loop
    ret

Test_Cache_Nt:
    xor rax, rax
    align 64
.read_loop:
    vmovdqu ymm0, [rdx]
    vmovdqu ymm0, [rdx + 32]
    vmovdqu ymm0, [rdx + 64]
    vmovdqu ymm0, [rdx + 96]

    add rax, 128
    cmp rax, rcx
    jb .read_loop

    mov rax, rdx
.write_loop:
    vmovntdq [rax], ymm0
    vmovntdq [rax + 32], ymm0
    vmovntdq [rax + 64], ymm0
    vmovntdq [rax + 96], ymm0
    add rax, 128
    sub rcx, 128
    jnz .write_loop
    ret
