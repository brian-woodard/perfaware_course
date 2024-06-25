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
;  LISTING 152
;  ========================================================================

global Test_Cache

section .text

;
; NOTE(casey): These ASM routines are written for the Windows
; 64-bit ABI. They expect RCX to be the first parameter (the count),
; and in the case of MOVAllBytesASM, RDX to be the second
; parameter (the data pointer), R8 to be the third parameter (the mask).
;
; To use these on a platform
; with a different ABI, you would have to change those registers
; to match the ABI.
;

Test_Cache:
    xor rax, rax
    xor r10, r10
    mov r11, rdx
    align 64

.loop:
    vmovdqu ymm0, [r11]
    vmovdqu ymm0, [r11 + 32]
    vmovdqu ymm0, [r11 + 64]
    vmovdqu ymm0, [r11 + 96]
    vmovdqu ymm0, [r11 + 128]
    vmovdqu ymm0, [r11 + 160]
    vmovdqu ymm0, [r11 + 192]
    vmovdqu ymm0, [r11 + 224]

    add r10, 256
    and r10, r8

    mov r11, rdx
    add r11, r10

    add rax, 256
    cmp rax, rcx
    jb .loop
    ret
