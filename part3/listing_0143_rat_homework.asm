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
;  LISTING 143
;  ========================================================================

;
;  NOTE(casey): Regular Homework
;                  |  Dest  |  Src      |
    mov rax, 1    ;|   s1   |  s0       |
    mov rbx, 2    ;|   s3   |  s2       |
    mov rcx, 3    ;|   s5   |  s4       |
    mov rdx, 4    ;|   s7   |  s6       |
    add rax, rbx  ;|   s8   |  s1, s3   |
    add rcx, rdx  ;|   s9   |  s5, s7   |
    add rax, rcx  ;|   s10  |  s8, s9   |
    mov rcx, rbx  ;|   s11  |  s9, s3   |
    inc rax       ;|   s12  |  s10      |
    dec rcx       ;|   s13  |  s11      |
    sub rax, rbx  ;|   s14  |  s12, s3  |
    sub rcx, rdx  ;|   s15  |  s13, s7  |
    sub rax, rcx  ;|   s16  |  s14, s15 |

;
;  NOTE(casey): CHALLENGE MODE WITH ULTIMATE DIFFICULTY SETTINGS
;               DO NOT ATTEMPT THIS! IT IS MUCH TOO HARD FOR
;               A HOMEWORK ASSIGNMENT!1!11!!
;
top:
    pop rcx
    sub rsp, rdx
    mov rbx, rax
    shl rbx, 0
    not rbx
    loopne top
