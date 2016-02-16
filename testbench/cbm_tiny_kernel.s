
#include "cbm_tiny_kernel.h"

/* vim: set tabstop=12 softtabstop=12 shiftwidth=12 expandtab: */


/* addresses used to communicate between C++ and the kernel */
#define SEL REGISTERS_START + OFF_SELECT
#define IO REGISTERS_START + OFF_IO
#define ACK REGISTERS_START + OFF_ACK
#define BASICE REGISTERS_START + OFF_BASIC_ENTRY
#define SCREENX REGISTERS_START + OFF_SCREEN_WIDTH
#define SCREENY REGISTERS_START + OFF_SCREEN_HEIGHT
#define RAM_TOP REGISTERS_START + OFF_RAM_TOP
#define RAM_BOT REGISTERS_START + OFF_RAM_BOTTOM

/* Variables used by kernel */
#define RAM_TOP_CURRENT REGISTERS_START+REGISTERS_SIZE-2
#define RAM_BOT_CURRENT REGISTERS_START+REGISTERS_SIZE-4
#define VAR0 REGISTERS_START + REGISTERS_SIZE - 5
#define LAST_CHAR REGISTERS_START + REGISTERS_SIZE - 6

#define CBM_CR $0d
#define CBM_CLR $93

#define ASCII_NL $0a
#define ASCII_FF $0c

*=$fe00

not_implemented:
.(
            php
            pha
            lda #SELECT_NOT_IMPL
            sta SEL
            pla       ; just restore all registers...
            plp
            rts
.)

reset:
.(
            cld         ;  initial state of decimal flag is undefined - clear it
            lda RAM_BOT
            sta RAM_BOT_CURRENT       ; setup RAM bottom and top pointers
            lda RAM_BOT+1
            sta RAM_BOT_CURRENT+1
            lda RAM_TOP
            sta RAM_TOP_CURRENT
            lda RAM_TOP+1
            sta RAM_TOP_CURRENT+1
            pha             ; basic just can't start with a zero stack pointer
            pha
            jmp (BASICE)
.)

control_chars:
            .byte ASCII_NL, CBM_CR
            .byte ASCII_FF, CBM_CLR
control_chars_end:

#define control_count control_chars_end-control_chars

chrin          ; Read byte from default input
.(
            stx VAR0
            lda #SELECT_CHRIN
            sta SEL
io_loop:    lda IO
            beq io_loop
            inc ACK
            ldx #0
lookup:     cmp control_chars, x
            beq translate
            inx
            inx
            cpx #control_count
            bne lookup
            jmp return
translate:  lda control_chars+1, x
return:     sta LAST_CHAR
            ldx VAR0
            clc           ; clear carry - indicate success
            rts
.)

chrout         ; Write byte to default output. 
.(
            pha
            cmp #CBM_CR     ; workaround - the screen editor does not print CR
            bne start       ;  when return is pressed, so basic does print one.
            cmp LAST_CHAR   ;  But when a terminal is used via C++, a newline is
            bne start       ;  already printed upon pressing enter
            lda #0
            sta LAST_CHAR
            jmp return
start:      stx VAR0
            ldx #SELECT_CHROUT
            stx SEL
            ldx #0
lookup:     cmp control_chars+1, x
            beq translate
            inx
            inx
            cpx #control_count
            bne lookup
            jmp write
translate:  lda control_chars, x
write:      sta IO
            ldx VAR0
return:     pla
            clc           ; clear carry - indicate success
            rts
.)

screenxy      ; Fetch number of screen rows and columns.
.(
            ldx SCREENX
            ldy SCREENY
            rts
.)

memtop       ; Save or restore end address of BASIC work area.
.(
            bcc settop
            ldx RAM_TOP_CURRENT
            ldy RAM_TOP_CURRENT+1
            rts
settop:     stx RAM_TOP_CURRENT
            sty RAM_TOP_CURRENT+1
            rts
.)

membot       ; Save or restore start address of BASIC work area.
.(
            bcc setbot
            ldx RAM_BOT_CURRENT
            ldy RAM_BOT_CURRENT+1
            rts
setbot:     stx RAM_BOT_CURRENT
            sty RAM_BOT_CURRENT+1
            rts
.)

NMI_service_routine
.(
            pha
            txa
            pha
            tya
            pha

             ; todo: jiffy clock

            pla
            tay
            pla
            tax
            pla
            rti
.)

IRQ_service_routine
.(
            rti
.)

.dsb $ff81 - * , $ea

*=$ff81
            jmp not_implemented
*=$ff84
            jmp not_implemented
*=$ff87
            jmp not_implemented
*=$ff8a
            jmp not_implemented
*=$ff8d
            jmp not_implemented
*=$ff90
            jmp not_implemented
*=$ff93
            jmp not_implemented
*=$ff96
            jmp not_implemented
*=$ff99
            jmp memtop
*=$ff9c
            jmp membot
*=$ff9f
            jmp not_implemented
*=$ffa2
            jmp not_implemented
*=$ffa5
            jmp not_implemented
*=$ffa8
            jmp not_implemented
*=$ffab
            jmp not_implemented
*=$ffae
            jmp not_implemented
*=$ffb1
            jmp not_implemented
*=$ffb4
            jmp not_implemented
*=$ffb7
            jmp not_implemented
*=$ffba
            jmp not_implemented
*=$ffbd
            jmp not_implemented
*=$ffc0
            jmp not_implemented
*=$ffc3
            jmp not_implemented
*=$ffc6
            jmp not_implemented
*=$ffc9
            jmp not_implemented
*=$ffcc
            jmp not_implemented
*=$ffcf
            jmp chrin
*=$ffd2
            jmp chrout
*=$ffd5
            jmp not_implemented
*=$ffd8
            jmp not_implemented
*=$ffdb
            jmp not_implemented
*=$ffde
            jmp not_implemented
*=$ffe1
            jmp not_implemented
*=$ffe4
            jmp not_implemented
*=$ffe7
            jmp not_implemented
*=$ffea
            jmp not_implemented
*=$ffed
            jmp screenxy
*=$fff0
            jmp not_implemented
*=$fff3
            jmp not_implemented



*=$fff6
            .byte $52, $52, $42, $59 ; unused - bytes taken from the C=64 kernal

*=$fffa
            .word NMI_service_routine
*=$fffc
            .word reset
*=$fffe
            .word IRQ_service_routine
