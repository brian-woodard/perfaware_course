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

section .text

;
; NOTE(casey): This ASM routine is written for the Windows 64-bit ABI.
;
;    rcx: block count
;    rdx: read data pointer
;     r8: write data pointer
;     r9: write count
;

Test_Cache:
    xor rax, rax
    align 64
.loop:
    vmovdqu ymm0, [rdx]
    vmovdqu ymm0, [rdx + 32]
    vmovdqu ymm0, [rdx + 64]
    vmovdqu ymm0, [rdx + 96]

.inner_loop:
    vmovdqu [r8], ymm0
    vmovdqu [r8 + 32], ymm0
    vmovdqu [r8 + 64], ymm0
    vmovdqu [r8 + 96], ymm0
    ;vmovntdq [r8], ymm0
    ;vmovntdq [r8 + 32], ymm0
    ;vmovntdq [r8 + 64], ymm0
    ;vmovntdq [r8 + 96], ymm0
    sub r9, 128
    jb .inner_loop

    add rax, 128
    cmp rax, rcx
    jb .loop
    ret

